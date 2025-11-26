from typing import Dict, List
import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
import matplotlib.colors as mplcolor
import os
import numpy as np
from numpy.typing import NDArray

import sys

sys.path.append("../")

import styling as sty


def parse_columns(file: str, ncols: int = 2) -> np.ndarray:
    with open(file) as f:
        lines = f.readlines()
    ret = []
    for line in lines:
        split = line.split()
        if len(split) == ncols:
            try:
                ret.append([float(col) for col in split])
            except:  ## probably we are parsing a string; skip in this case
                continue
    return np.array(ret)


def read_dir(path: str, dir: str) -> List[NDArray]:
    aux = []
    for dir in os.listdir(path):
        fresco = f"{path}/{dir}/fort.202"
        if os.path.exists(fresco):
            it = dir.find("_")
            beta = dir[it + 1 :]
            # models[float(beta)] = parse_columns(fresco)
            aux.append((beta, parse_columns(fresco)))
    aux = sorted(aux, key=lambda x: x[0])
    return [b for a, b in aux]


paths = ["../../Fits/dd/Search/g1_Daeh/l2", "../../Fits/dd/Search/g3_Daeh/l3"]
arrays = [read_dir(path, "") for path in paths]
labels = [r"$2^+$", r"$3^-$"]

colors = [
    plt.get_cmap("Oranges")(np.linspace(0.25, 1, len(arrays[0]))),
    plt.get_cmap("Greens")(np.linspace(0.25, 1, len(arrays[1]))),
]
fig, axs = plt.subplots(1, 2, figsize=(9, 3.5), layout="constrained")
for i, ax in enumerate(axs):
    ax: Axes
    top = arrays[i][-1]
    bottom = arrays[i][0]
    # ax.fill_between(
    #     bottom[:, 0], bottom[:, 1], top[:, 1], color=f"C{i + 1}", alpha=0.75
    # )
    ax.set_prop_cycle(color=colors[i])
    for line in arrays[i]:
        ax.plot(line[:, 0], line[:, 1])
    ax.set_xlim(5, 50)

phys.utils.annotate_subplots(axs, x=0.825)

pos = [[(20, 60), (20, 0)], [(30, 10), (30, 0.5)]]
for i, ax in enumerate(axs):
    ax.annotate(
        labels[i],
        xy=(0.825, 0.825),
        xycoords="axes fraction",
        **sty.ann,
        fontweight="bold",
    )
    ax.annotate("", xy=pos[i][0], xytext=pos[i][1], arrowprops=sty.arrowprops)
    ax.annotate(
        rf"$\beta_{{{i + 2}}} \uparrow$",
        xy=(pos[i][0][0] + 3, pos[i][0][1] - 2),
        **sty.ann,
    )


fig.supxlabel(
    r"$\theta_{CM}$ [$\circ$]", x=0.55, fontsize=plt.rcParams["axes.labelsize"]
)
fig.supylabel(
    r"$d\sigma/d\Omega$ [$mb\cdot sr^{-1}$]", fontsize=plt.rcParams["axes.labelsize"]
)

fig.savefig(sty.thesis + "inelastic_explanation.pdf", dpi=300)
plt.show()
