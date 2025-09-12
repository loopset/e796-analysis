from collections import defaultdict
from re import A
from typing import Dict, List, Tuple
import pyphysics as phys
import pickle
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import matplotlib.pyplot as plt

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty


# Read reanalysis data
reana: phys.SMDataDict = {}
with open("../../Fits/dt/Mairle_18Odt/Outputs/reanalysis.pkl", "rb") as f:
    reana = pickle.load(f)

mairle: phys.SMDataDict = {}
with open("../../Fits/dt/Mairle_18Odt/Outputs/mairle.pkl", "rb") as f:
    mairle = pickle.load(f)


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
        maxEx = 15
        for ex in np.arange(maxEx, step=0.2):  # type: ignore
            aux: phys.SMDataDict = {}
            aux[q] = [e for e in all if e.Ex <= ex]  # type: ignore
            try:
                cent = dt.get_centroids(aux)[q]
            except ZeroDivisionError:
                continue
            retEx[q].append((ex, cent))

    return [retSte, retEx]


fig, axs = plt.subplots(1, 2)
for i, data in enumerate([reana, mairle]):
    for j, dic in enumerate(do_cumulative(data)):
        for q, vals in dic.items():
            x, y = zip(*vals)
            axs[j].errorbar(
                unp.nominal_values(x),
                unp.nominal_values(y),
                xerr=unp.std_devs(x),
                yerr=unp.std_devs(y),
                color=sty.barplot[q]["ec"],
                **sty.errorbar_line,
                label=q.format() if i == 0 else None,
                alpha= 1 if i == 0 else 0.5
            )

axs[0].set_xlabel("Strength [%]")
axs[1].set_xlabel("Ex cutoff [MeV]")
# Common settings
for ax in axs:
    ax.legend()
    ax.set_ylabel("Centroid [MeV]")

# Plot things
ax = axs[0]
# 0p1/2
ax.axvline(61, color=sty.barplot[dt.qp12]["ec"], ls="--")
#0p3/2
ax.axvline(14, color=sty.barplot[dt.qp32]["ec"], ls="--")
# Ex cut
ax = axs[1]
ax.axvline(12.5, color="crimson", ls="--")

fig.tight_layout()
plt.show()
