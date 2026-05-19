import sys
from collections import defaultdict
from typing import Dict, List

import matplotlib.pyplot as plt
import numpy as np
import pyphysics as phys
from matplotlib.axes import Axes
from matplotlib.lines import Line2D

sys.path.append("./")
sys.path.append("../")
import styling as sty

qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
qd52 = phys.QuantumNumbers(0, 2, 2.5)
qs12 = phys.QuantumNumbers(1, 0, 0.5)

subdirs = ["gs"]
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

qsub = [qs12, qd52, qp12, qp32]

fig, ax = plt.subplots(1, 1, figsize=(5, 4), constrained_layout=True)
ax: Axes
ax.set_yscale("log")
xmin = 0
xmax = 50
for i, q in enumerate(qsub):
    for idx, file in enumerate(files[q]):
        spline = phys.create_spline3(file[:, 0], file[:, 1])
        x = np.linspace(xmin, xmax, 200)
        y = spline(x)
        ax.plot(x, y, color=sty.barplot[q]["ec"], ls=ls[idx], label=q.format())
ax.set_xlim(xmin, xmax)
ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
# ax.set_ylim(2e-1, 20)
ax.set_ylabel(r"$d\sigma/d\Omega$ [$mb\cdot sr^{-1}$]")
ax.legend()
phys.utils.apply_log_formatter(ax)

fig.savefig("./xs_slides.png", dpi=300)
plt.show()
