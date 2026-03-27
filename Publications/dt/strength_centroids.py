import copy
from typing import Dict, List, Tuple, Union
import pyphysics as phys
import hist
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import pandas as pd
from matplotlib.lines import Line2D
import pickle
import sys

sys.path.append("./")
sys.path.append("../")
import dt
import styling as sty

## Experimental dataset
exp = dt.build_sm()

theos = dt.build_theos(gated=False)
asf = hist.axis.Regular(200, 0, 1, name="sf", label="C2S")
# amodel = hist.axis.StrCategory(["0", "1", "2"], name="m")
aq = hist.axis.StrCategory(
    [
        dt.qd52.format_simple(),
        dt.qs12.format_simple(),
        dt.qp12.format_simple(),
        dt.qp32.format_simple(),
    ],
    name="q",
)
hsf = hist.Hist(asf, aq)

for i, theo in enumerate(theos):
    h = hist.Hist.new.Reg(200, 0, 1.5, label=r"$C^2S$").Double()
    if i == 1:  # fill distribution of C2S
        for q, lis in theo.data.items():
            for el in lis:
                hsf.fill(sf=un.nominal_value(el.SF), q=q.format_simple())
    theo.set_min_SF(0.0)


strens = []
for d in [exp, *[m.data for m in theos]]:
    strens.append(dt.get_strengths(d))

cents = []
for d in [exp, *[m.data for m in theos]]:
    cents.append(dt.get_centroids(d))

# Write GATED to disk
with open("./Inputs/strength_centroids_nogates.pkl", "wb") as f:
    pickle.dump((strens, cents), f)

# Compute directly gaps, assuming no population in (d,p) reactions
# as Bea's paper proves
gaps = []
for i, d in enumerate(cents):
    gap = d[dt.qp32] - d[dt.qp12]
    gaps.append(gap)
    print(
        "->", "Experimental" if i == 0 else "Mod SFO-tls", " gap : ", f"{gap:.2f} MeV"
    )


def do_cumulative(
    data: phys.SMDataDict,
) -> Dict[phys.QuantumNumbers, Tuple[list, list, list]]:
    ret = {}
    for q in [dt.qd52, dt.qs12, dt.qp12, dt.qp32]:
        lis = data.get(q, None)
        if lis is None:
            return {}
        ret[q] = ()
        ex = np.arange(0, dt.maxExtheo + 5, 0.1)
        y = []
        z = []
        for e in ex:
            aux: phys.SMDataDict = {}
            aux[q] = [pair for pair in lis if un.nominal_value(pair.Ex) <= e]
            ste = dt.get_strengths(aux)[q]  # / q.degeneracy() * 100  # type: ignore
            cent = dt.get_centroids(aux)[q]
            y.append(ste)
            z.append(cent)
        ret[q] = (ex, y, z)
    return ret


cums = []
for d in [exp, *[m.data for m in theos]]:
    cums.append(do_cumulative(d))

labels = ["Exp", "SFO-tls", "Mod1", "Mod2"]
ls = ["-", "--", "dotted", "dashdot"]

# Declare list with states
qs = [dt.qd52, dt.qs12, dt.qp12, dt.qp32]
maxste = [4, 2, 2, 4]

## Plot strengths
fig, axs = plt.subplots(1, 4, figsize=(11, 3), constrained_layout=True)
for i, q in enumerate(qs):
    ax: mplaxes.Axes = axs[i]
    color = sty.barplot.get(q)["ec"]  # type: ignore
    for j, cum in enumerate(cums):
        tup = cum.get(q, None)
        if tup is None:
            continue
        x, y, _ = tup
        y = np.array(y) / maxste[i] * 100
        ax.plot(
            unp.nominal_values(x),
            unp.nominal_values(y),
            ls=ls[j],
            color=color,  # type: ignore
        )
        if j == 0:
            centers = unp.nominal_values(y)
            bands = unp.std_devs(y)
            ax.fill_between(
                unp.nominal_values(x),
                centers - bands,
                centers + bands,
                color=color,
                alpha=0.2,
            )
    # Annotate
    ax.annotate(
        rf"$\nu${q.format()}",
        xy=(0.85, 0.1),
        xycoords="axes fraction",
        color=color,
        **{**sty.ann, "fontsize": 16},
    )
    if i == 0:
        ax.set_ylabel(r"$\sum$C$^2$S / max [%]")
    ax.set_xlim(0, dt.maxExtheo + 2)
    ax.set_ylim(0)
    ax.axvline(dt.maxEx, ls="--", color="orange")

# Axis settings
handles = []
for l, label in zip(ls, labels):
    line = Line2D([], [], ls=l, color="grey", label=label)
    handles.append(line)
axs[0].legend(handles=handles)

fig.supxlabel(r"$E_{x}$ [MeV]", fontsize=plt.rcParams["axes.labelsize"])

# Save
fig.savefig(sty.thesis + "cdf_strength.pdf", dpi=300)

## Plot distributions of C2S
# fig, ax = plt.subplots()
# hsf.plot()


# Build df
stedf = pd.DataFrame()
for q in qs:
    dic = {"q": q.format_simple()}
    for i, lis in enumerate(strens):
        dic[labels[i]] = [lis[q]]  # type: ignore
    stedf = pd.concat([stedf, pd.DataFrame(dic)], ignore_index=True)

for i, col in enumerate(labels):
    stedf[f"{col} ratio"] = stedf[col] / maxste


def fmt(x):
    if isinstance(x, str):
        return x
    if hasattr(x, "nominal_value"):
        return f"{x:.2uS}"
    else:
        return f"{x:.2f}"


stedf = stedf.map(fmt)
print(stedf)

################################### Centroids
# Theo models up to the IAS Ex and threshold in C2S
gatedtheos = dt.build_theos(gated=True)
gatedstrens = [strens[0]]
gatedcents = [cents[0]]
for d in [m.data for m in gatedtheos]:
    gatedstrens.append(dt.get_strengths(d))
    gatedcents.append(dt.get_centroids(d))

# Write GATED to disk
with open("./Inputs/strength_centroids.pkl", "wb") as f:
    pickle.dump((gatedstrens, gatedcents), f)


# plt.close("all")
# Independent figure with centroids
fig, axs = plt.subplots(3, 1, figsize=(6, 5.5), sharex=True, constrained_layout=True)


def plot_centroids(
    ste, cent, ax: mplaxes.Axes, withLabel=False, **kwargs
) -> List[Tuple[float, float]]:
    ret = []
    width = 0.5
    for q, x in cent.items():
        if q == phys.QuantumNumbers(0, 2, 1.5):
            continue
        y = ste[q]
        ax.bar(
            x=un.nominal_value(x),  # type: ignore
            height=un.nominal_value(y),  # type: ignore
            yerr=un.std_dev(y) if un.std_dev(y) > 0 else None,
            width=width,
            align="center",
            label=q.format() if withLabel else None,
            **sty.barplot[q],
            **kwargs,
        )
        ret.append((x, y))
    ax.set_ylim(0)
    return ret


# Experimental
ax: mplaxes.Axes = axs[0]
plot_centroids(strens[0], cents[0], ax, withLabel=True)

# Theo WITH threshold
for i, ax in enumerate(axs[1:], start=1):
    plot_centroids(gatedstrens[i], gatedcents[i], ax)
# # Theo withouth threshold
# for i, ax in enumerate(axs[1:], 1):
#     plot_centroids(strens[i], cents[i], ax, ls="--", alpha=0.75)

# Common settings
for i, ax in enumerate(axs):
    ax.set_ylim(0, 4)
    ax.set_xlim(ax.get_xlim()[0], 15)
    if i == 0:
        leg = ax.legend(loc="center right")
        # leg.set_in_layout(False)
    # Annotations
    pos = (0.5, 0.85)
    ax.annotate(
        labels[i], xy=pos, xycoords="axes fraction", **sty.ann, fontweight="bold"
    )

fig.supxlabel(
    r"$\langle E_{x}\rangle$ [MeV]", x=0.55, fontsize=plt.rcParams["axes.labelsize"]
)
fig.supylabel(r"$\sum C^2S$", fontsize=plt.rcParams["axes.labelsize"])

fig.savefig(sty.thesis + "centroids.pdf", dpi=300)

plt.show()
