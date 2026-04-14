from shutil import which

import pyphysics as phys
import uncertainties as un
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.axes import Axes
import pickle
from typing import List, Dict, Union
from matplotlib.patches import Rectangle
import sys

sys.path.append("./")
sys.path.append("../")
import styling as sty
import dt

# Types
DictType = Dict[phys.QuantumNumbers, float | un.Variable]

# Read data
espe_gated: List[DictType] = []
with open("./Inputs/espes_gaps.pkl", "rb") as f:
    espe_gated = pickle.load(f)[0]
espe_nogated: List[DictType] = []
with open("./Inputs/espes_gaps_nogates.pkl", "rb") as f:
    espe_nogated = pickle.load(f)[0]

# Labels
labels = ["Exp", "SFO-tls", "Mod1"]


def plot(ax: Axes, data: DictType, idx: int, qargs: List | None = None, **kwargs):
    # Some options
    width = 0.7
    qs = [dt.qs12, dt.qd52, dt.qp12, dt.qp32]
    if qargs is not None:
        qs = qargs
    for q in qs:
        espe = data.get(q, None)
        if espe is None:
            continue
        y = un.nominal_value(espe)
        uy = un.std_dev(espe)
        # Color
        color = sty.barplot[q]["ec"]
        # Line
        ax.plot(
            [idx - width / 2, idx + width / 2],
            [y] * 2,
            lw=1.25,
            color=color,
            label=f"{q.format()}" if idx == 0 else None,
            **kwargs,
        )
        # Rectangle patch
        if uy > 0:
            rec = Rectangle(
                xy=(idx - width / 2, y - uy),
                width=width,
                height=2 * uy,
                ec="none",
                fc=color,
                alpha=0.25,
            )
            ax.add_patch(rec)


fig, ax = plt.subplots(1, 1, figsize=(5, 3.25), constrained_layout=True)
ax: Axes

# Gated
for i in [0, 1, 2]:
    plot(ax, data=espe_gated[i], idx=i)
# NO GATED
for i in [1, 2]:
    plot(ax, data=espe_nogated[i], idx=i, ls="--", qargs=[dt.qp12, dt.qp32])

# Axis settings
ax.set_xticks(list(range(len(labels))), labels)
ax.tick_params(axis="x", which="both", bottom=False, top=False)
ax.set_ylim(-25, 0)

# # Grid?
# ax.grid(axis="y", which="major", lw=0.5)
# ax.grid(axis="y", which="minor", ls="--", lw=0.5)
# Bound limit
# ax.axhline(0, ls="--", color="black", lw=0.75)

# # Legend
# ax.legend(
#     loc="upper right",
#     frameon=True,
#     fancybox=True,
#     borderpad=0.2,
#     labelspacing=0.3,
#     handlelength=1.5,
#     columnspacing=1,
#     ncols=2,
#     draggable=True,
# )

# Annotations
ax.set_xlim(-0.85)
for q in [dt.qs12, dt.qd52, dt.qp12, dt.qp32]:
    color = sty.barplot[q]["ec"]
    y = un.nominal_value(espe_gated[0][q])
    ax.annotate(
        f"{q.format()}",
        xy=(-0.6, y),
        ha="center",
        va="center",
        fontsize=14,
        color=color,
    )

ax.set_ylabel("ESPE [MeV]")

fig.savefig("../paper/Outputs/espes.pdf", dpi=300)

plt.show()
