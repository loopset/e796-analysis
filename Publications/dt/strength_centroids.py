import copy
import enum
from typing import Dict, List, Tuple, Union
import pyphysics as phys
import hist
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
from matplotlib.patches import ConnectionPatch
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import pandas as pd
from collections import defaultdict
from matplotlib.lines import Line2D
import sys

sys.path.append("./")
sys.path.append("../")
import dt
import styling as sty

## Experimental dataset
exp = dt.build_sm()

# Unmodified SFO-tls
sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
    ]
)
# Modified1 SFO-tls
sfo1 = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)
# Modified2 SFO-tls
sfo2 = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1n.txt",
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1p.txt",
    ]
)

expMaxE = 15.1
maxE = 25
# sfo.set_max_Ex(16.5)
# sfo2.set_max_Ex(14.75)
theos = [sfo, sfo1, sfo2]
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
    theo.set_max_Ex(maxE)
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
        ex = np.arange(0, maxE, 0.1)
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
        **sty.ann,
    )
    if i == 0:
        ax.set_ylabel(r"$\sum$C$^2$S / max [%]")
    ax.set_xlim(0, maxE)
    ax.set_ylim(0)
    ax.axvline(expMaxE, ls="--", color="orange")

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
# Theo models up to same Ex and threshold in C2S
gatedtheos = []
for theo in theos:
    clone = copy.deepcopy(theo)
    clone.set_max_Ex(16.5)
    clone.set_min_SF(0.04)
    gatedtheos.append(clone)
gatedstrens = []
gatedcents = []
for d in [m.data for m in gatedtheos]:
    gatedstrens.append(dt.get_strengths(d))
    gatedcents.append(dt.get_centroids(d))

# plt.close("all")
# Independent figure with centroids
fig = plt.figure(figsize=(8.5, 4.5), constrained_layout=True)
# use 6 columns: bottom three subplots each take 2 cols; top center spans 4 cols (1:5)
gs = fig.add_gridspec(2, 6, height_ratios=[1, 1])
axtop = fig.add_subplot(gs[0, 1:5])
axsbot = [
    fig.add_subplot(gs[1, 0:2], sharex=axtop, sharey=axtop),
    fig.add_subplot(gs[1, 2:4], sharex=axtop, sharey=axtop),
    fig.add_subplot(gs[1, 4:6], sharex=axtop, sharey=axtop),
]


def plot_centroids(ste, cent, ax: mplaxes.Axes, **kwargs) -> List[Tuple[float, float]]:
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
            label=q.format(),
            **sty.barplot[q],
            **kwargs,
        )
        ret.append((x, y))
    ax.set_ylim(0)
    return ret


# Top: experimental
plot_centroids(strens[0], cents[0], axtop)
# Bottom: models
# Gated
for i in range(3):
    ax = axsbot[i]
    plot_centroids(gatedstrens[i], gatedcents[i], ax)
# Full strength
for i in range(1, 4):
    ax = axsbot[i - 1]
    plot_centroids(strens[i], cents[i], ax, ls="--", alpha=0.75)

# Axis settings
axtop.set_ylim(0, 4)
axtop.set_ylabel(r"$\sum C^2S$")
# axsbot[0].set_ylabel(r"$\sum C^2S$")
axsbot[1].set_xlabel(r"$E_{x}$ [MeV]")
leg = axtop.legend(loc="center left", bbox_to_anchor=(1, 0.5))
leg.set_in_layout(False)

# Annotations
pos = (0.5, 0.85)
axtop.annotate(
    labels[0], xy=pos, xycoords="axes fraction", **sty.ann, fontweight="bold"
)
for i, axb in enumerate(axsbot):
    axb.annotate(
        labels[i + 1], xy=pos, xycoords="axes fraction", **sty.ann, fontweight="bold"
    )

fig.savefig(sty.thesis + "centroids.pdf", dpi=300)

plt.show()
