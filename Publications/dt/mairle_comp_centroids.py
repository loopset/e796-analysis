from collections import defaultdict
from re import A
from typing import Dict, List, Tuple

from scipy.fft import dst
import pyphysics as phys
import pickle
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty


# Read reanalysis data
reana: phys.SMDataDict = {}
with open("../../Fits/dt/Mairle_18Odt/Outputs/reanalysis.pkl", "rb") as f:
    reana = pickle.load(f)

# Original data
mairle: phys.SMDataDict = {}
with open("../../Fits/dt/Mairle_18Odt/Outputs/mairle.pkl", "rb") as f:
    mairle = pickle.load(f)

# Read theoretical
files = [
    "../../Fits/dt/Mairle_18Odt/Inputs/SFO-tls/mod1.pkl",
    "../../Fits/dt/Mairle_18Odt/Inputs/SFO-tls/mod2.pkl",
]
theos: List[phys.SMDataDict] = []
for file in files:
    with open(file, "rb") as f:
        sm = pickle.load(f)
        theos.append(sm.data)

# Build models
models = [mairle, reana] + theos
labels = ["Paper", "Reana", "Theo1", "Theo2"]

# Styles
styles = {
    "ls": ["-", "--", "-.", ":"],
    "ms": ["o", "s", "D", "^"],
    "hs": ["", "////", "xxx", "++"],
}


def do_cumulative(data: phys.SMDataDict) -> List:
    retSte: Dict[phys.QuantumNumbers, List] = defaultdict(list)
    for q in [dt.qp12, dt.qp32]:
        all = data[q]
        for i in range(len(all)):
            aux: phys.SMDataDict = {}
            aux[q] = all[: i + 1]
            ste = dt.get_strengths(aux)[q] / q.degeneracy() * 100  # type: ignore
            cent = dt.get_centroids(aux)[q]
            retSte[q].append((ste, cent))

    retEx: Dict[phys.QuantumNumbers, List] = defaultdict(list)
    for q in [dt.qp12, dt.qp32]:
        all = data[q]
        # Max in Ex
        # maxEx = un.nominal_value(max(all, key=lambda x: x.Ex).Ex)  # type: ignore
        maxEx = 19
        for ex in np.arange(maxEx, step=0.2):  # type: ignore
            aux: phys.SMDataDict = {}
            aux[q] = [e for e in all if e.Ex <= ex]  # type: ignore
            try:
                cent = dt.get_centroids(aux)[q]
            except ZeroDivisionError:
                continue
            retEx[q].append((ex, cent))

    return [retSte, retEx]


cums = []
fig, axs = plt.subplots(2, 2, figsize=(8, 6))
for i, val in enumerate(models):
    cums.append(do_cumulative(val))
    for j, dic in enumerate(cums[-1]):
        for q, vals in dic.items():
            x, y = zip(*vals)
            axs[0, j].plot(
                unp.nominal_values(x),
                unp.nominal_values(y),
                # xerr=unp.std_devs(x),
                # yerr=unp.std_devs(y),
                color=sty.barplot[q]["ec"],
                label=labels[i] if q == dt.qp12 else None,
                ls=styles["ls"][i],
                marker="none",
                # capsize=0,
            )


axs[0, 0].set_xlabel("Strength [%]")
axs[0, 1].set_xlabel(r"E$_{x}$ cutoff [MeV]")
# Common settings
for ax in axs.flat[:2]:
    ax.set_ylabel("Centroid [MeV]")
axs[0, 1].legend(loc="lower left", bbox_to_anchor=(1.05, 0.25), ncols=1)

# Plot things
ax: mplaxes.Axes = axs[0, 0]
# 0p1/2
ourp12 = 61  # percent
ax.axvline(ourp12, color=sty.barplot[dt.qp12]["ec"], ls="--")
# 0p3/2
ourp32 = 14  # percent
ax.axvline(ourp32, color=sty.barplot[dt.qp32]["ec"], ls="--")
# Ex cut
ax = axs[0, 1]
ourMaxEx = 15.5 # MeV + 0.5 just in case
ax.axvline(ourMaxEx, color="crimson", ls="--")


# Find closest
closeSte = []
closeEx = []
gapSte = []
gapEx = []
for i, cum in enumerate(cums):
    ste, ex = cum
    dic = {}
    # Strength
    # 0p12
    dic[dt.qp12] = min(
        ste[dt.qp12], key=lambda x: abs(un.nominal_value(x[0] - ourp12))
    )[1]
    # 0p32
    dic[dt.qp32] = min(
        ste[dt.qp32], key=lambda x: abs(un.nominal_value(x[0] - ourp32))
    )[1]
    gapSte.append(dic[dt.qp32] - dic[dt.qp12])
    # Ex
    dex = {}
    dex[dt.qp12] = min(
        ex[dt.qp12], key=lambda x: abs(un.nominal_value(x[0] - ourMaxEx))
    )[1]
    dex[dt.qp32] = min(
        ex[dt.qp32], key=lambda x: abs(un.nominal_value(x[0] - ourMaxEx))
    )[1]
    gapEx.append(dex[dt.qp32] - dex[dt.qp12])
    # Append
    closeSte.append(dic)
    closeEx.append(dex)

# Plot centroids
width = 0.35
ax: mplaxes.Axes = axs[1, 0]
xlabels = ["Strength", r"E$_{x}$"]
for i, cond in enumerate([closeSte, closeEx]):
    for j, model in enumerate(cond):
        for q, val in model.items():
            ax.barh(
                un.nominal_value(val),
                left=i - width / 2,
                width=width,
                height=0.1,
                color=sty.barplot[q]["ec"],
                alpha=1 if j == 0 else 0.5,
                hatch=styles["hs"][j],
            )
            ax.annotate(
                f"{val:.2uS}" if isinstance(val, un.UFloat) else f"{val:.2f}",
                xy=(i + width / 2, un.nominal_value(val)),
                ha="left",
                va="center",
            )
ax.set_xticks([0, 1], xlabels)
ax.set_xlim(-0.75, 1.75)
ax.set_xlabel("Condition")
ax.set_ylabel("Centroid [MeV]")

# And gap
ax: mplaxes.Axes = axs[1, 1]
for i, cond in enumerate([gapSte, gapEx]):
    for j, val in enumerate(cond):
        ax.barh(
            un.nominal_value(val),
            left=i - width / 2,
            width=width,
            height=0.1,
            color="orange",
            alpha=1 if j == 0 else 0.5,
            hatch=styles["hs"][j],
            label=labels[j] if (i == 0) else None,
        )
        ax.annotate(
            f"{val:.2uS}" if isinstance(val, un.UFloat) else f"{val:.2f}",
            xy=(i + width / 2, un.nominal_value(val)),
            ha="left",
            va="center",
        )
ax.set_xticks([0, 1], xlabels)
ax.set_xlim(-0.75, 1.75)
ax.set_xlabel("Condition")
ax.set_ylabel("N = 6 gap [MeV]")
axs[1, 1].legend(loc="lower left", bbox_to_anchor=(1.05, 0.25), ncols=1)

# Gap 20O
# Experimental gap
ax.axhline(3.79, ls="--", color="dodgerblue", lw=1.5)
ax.annotate(
    r"Exp $^{20}$O gap",
    xy=(-0.25, 3.95),
    ha="center",
    va="center",
    fontsize=12,
    color="dodgerblue",
)

# Theoretical gap
ax.axhline(4.35, ls="--", color="crimson", lw=1.5)
ax.annotate(
    r"SFO-tls-2 $^{20}$O gap",
    xy=(-0.25, 4.5),
    ha="center",
    va="center",
    fontsize=12,
    color="crimson",
)

# # Print
# datanames = ["Reanalysis", "Paper"]
# for i, cond in enumerate([gapSte, gapEx]):
#     print("======", labels[i], "=====")
#     for j, data in enumerate(cond):
#         print(f"  {datanames[j]} : {data:.2f}")


fig.tight_layout()
# fig.savefig("./Outputs/mairle_comparison.pdf", dpi=300)
plt.show()
