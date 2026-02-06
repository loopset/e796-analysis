from collections import defaultdict
from typing import Dict, List
import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
import numpy as np
from matplotlib.lines import Line2D

import sys

sys.path.append("./")
sys.path.append("../")
import styling as sty

qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
qd52 = phys.QuantumNumbers(0, 2, 2.5)
qs12 = phys.QuantumNumbers(1, 0, 0.5)

subdirs = ["gs", "ex_5", "ex_10"]
ls = ["solid", "dashed", "dotted"]
maps = {
    qp12: 203,
    qp32: 205,
    qd52: 204,
    qs12: 202,
}

files: Dict[phys.QuantumNumbers, List] = defaultdict(list)

for q, fort in maps.items():
    for sub in subdirs:
        file = f"./Inputs/xs/{sub}/fort.{fort}"
        data = phys.parse_txt(file)
        files[q].append(data)

qsub = [qp32, qp12, qd52, qs12]

fig, axs = plt.subplots(1, 4, figsize=(11, 3), constrained_layout=True)
xmin = 0
xmax = 40
for i, q in enumerate(qsub):
    ax: Axes = axs[i]
    ax.set_yscale("log")
    for idx, file in enumerate(files[q]):
        spline = phys.create_spline3(file[:, 0], file[:, 1])
        x = np.linspace(xmin, xmax, 200)
        y = spline(x)
        ax.plot(x, y, color=sty.barplot[q]["ec"], ls=ls[idx])
    if i == 1 or i == 2:
        ax.tick_params(axis="y", labelleft=False)
    if i == (len(qsub) - 1):
        ax.yaxis.tick_right()
        ax.yaxis.set_ticks_position("both")

    ax.set_xlim(xmin, xmax)
    ax.set_ylim(2e-1, 20)
    ax.annotate(
        q.format(),
        xy=(0.5, 0.875),
        xycoords="axes fraction",
        **{**sty.ann, "fontsize": 16},
        color=sty.barplot[q]["ec"],
    )
    # Second legend
    ex0 = Line2D([], [], color="black", ls="solid", label=r"$E_{x} = 0$ MeV")
    ex5 = Line2D([], [], color="black", ls="dashed", label=r"$E_{x} = 5$ MeV")
    ex10 = Line2D([], [], color="black", ls="dotted", label=r"$E_{x} = 10$ MeV")
    if i == 0:
        ax.legend(handles=[ex0, ex5, ex10], loc="center right", fontsize=12)
        ax.set_ylabel(r"$d\sigma/d\Omega$ [$mb\cdot sr^{-1}$]")

    phys.utils.apply_log_formatter(ax)

fig.supxlabel(r"$\theta_{CM}$ [$\circ$]", fontsize=plt.rcParams["axes.labelsize"])
fig.savefig(sty.thesis + "xs_example.pdf", dpi=300)
plt.show()
