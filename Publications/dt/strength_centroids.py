from cProfile import label
from typing import Dict, List, Tuple, Union
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
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
for theo in theos:
    theo.set_max_Ex(maxE)
    theo.set_min_SF(0)


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
        ex = np.arange(0, maxE, 0.25)
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

ls = ["-", "--", "dotted", "dashdot"]

## Plot
fig, axs = plt.subplots(1, 2, figsize=(12, 4.5), constrained_layout=True)
# Strengths
ax: mplaxes.Axes = axs[0]
for i, cum in enumerate(cums):
    for q in [dt.qd52, dt.qs12, dt.qp12, dt.qp32]:
        tup = cum.get(q, None)
        if tup is None:
            continue
        x, y, _ = tup
        ax.plot(
            unp.nominal_values(x),
            unp.nominal_values(y),
            ls=ls[i],
            color=sty.barplot.get(q)["ec"],  # type: ignore
        )
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel(r"$\sum$C$^2$S")

# Centroids
ax = axs[1]
for i, cum in enumerate(cums):
    for q in [dt.qd52, dt.qs12, dt.qp12, dt.qp32]:
        tup = cum.get(q, None)
        if tup is None:
            continue
        x, _, y = tup
        ax.plot(
            unp.nominal_values(x),
            unp.nominal_values(y),
            ls=ls[i],
            color=sty.barplot.get(q)["ec"],  # type: ignore
        )
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel(r"$\langle E_{x}\rangle$ [MeV]")

# Plot cutoff
for ax in axs:
    ax.axvline(expMaxE, color="crimson", lw=1.25, label="Exp. cutoff")
    ax.axvline(
        phys.Particle("19O").get_sn(),
        color="purple",
        ls="dotted",
        lw=1.25,
        label=r"$S_{n}$",
    )

handles = []
labels = ["Exp", "SFO-tls", "Mod1", "Mod2"]
for l, label in zip(ls, labels):
    line = Line2D([], [], ls=l, color="grey", label=label)
    handles.append(line)
axs[0].legend(handles=handles)

for ax in axs:
    ax.set_ylim(0)


# Independent figure with centroids
fig, ax = plt.subplots(figsize=(7, 4))
width = 0.3  # MeV
for i, (ste, cent) in enumerate(zip(strens, cents)):
    for q, y in ste.items():
        if q == phys.QuantumNumbers(0, 2, 1.5):
            continue
        print(f"Centroid {q.format_simple()} = {cent[q]}")
        print(f"  Strength = {ste[q]}")
        ax.bar(
            x=un.nominal_value(cent[q]),
            height=un.nominal_value(y),
            yerr=un.std_dev(y) if i == 0 else None,
            width=width,
            align="center",
            label=q.format() if i == 0 else None,
            alpha=1 if i == 0 else 0.5,
            ls=ls[i],
            **sty.barplot[q],
        )
ax.legend()
ax.set_ylim(0)
ax.set_xlabel(r"Centroids [MeV]")
ax.set_ylabel(r"$\sum$C$^2$S")
ax.annotate(
    "Dimmed: Mod2 SFO-tls",
    xy=(0.4, 0.8),
    xycoords="axes fraction",
    ha="center",
    va="center",
    fontsize=14,
)

fig.tight_layout()
plt.show()
