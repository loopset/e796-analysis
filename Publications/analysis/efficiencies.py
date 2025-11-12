import pyphysics as phys
import awkward as ak
from pyphysics.root_interface import parse_tf1, parse_th1, parse_tgraph
import hist
import uproot
import re

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.ticker as mpltick
import matplotlib.colors as mpcolor
import matplotlib.lines as mplines
import uncertainties as un
import uncertainties.unumpy as unp
from scipy.constants import c as lightSpeed
from scipy.odr import ODR, Model, RealData

import sys

import ROOT as r

sys.path.append("../")
import styling as sty

# Read the data
pairs = [("1H", "1H"), ("1H", "2H"), ("2H", "2H"), ("2H", "3H")]
labels = ["(p,p)", "(p,d)", "(d,d)", "(d,t)"]


def get_simu_files(target: str, light: str) -> list:
    import re, os

    p = re.compile(rf"tree_20O_{target}_{light}_(\d+\.\d+)_nPS_0_pPS_0\.root$")
    path = "/media/Data/E796v2/Simulation/Outputs/juan_RPx/"
    files = [f"{path}/{f}" for f in os.listdir(path) if p.match(f)]
    return sorted(files, key=lambda x: float(p.search(os.path.basename(x)).group(1)))


# Read!
effs = [[] for i in range(len(pairs))]
for i, (target, light) in enumerate(pairs):
    files = get_simu_files(target, light)
    print("===================")
    for file in files:
        print(file)
        data = r.TFile(file)  # type: ignore
        g = parse_tgraph(data.Get("eff").CreateGraph())
        effs[i].append(g)

# Plot
fig, axs = plt.subplots(1, 2, figsize=(10, 4))

# Elastic factor
elfactor = 3

# Cmaps
cmaps = [
    plt.colormaps.get_cmap("Blues"),
    plt.colormaps.get_cmap("Reds"),
    plt.colormaps.get_cmap("Oranges"),
    plt.colormaps.get_cmap("Greens"),
]

# Proton
ax: mplaxes.Axes = axs[0]
for i, lis in enumerate(effs[:2]):
    factor = 1 if i != 0 else elfactor
    line = None
    for k, eff in enumerate([lis[0], lis[-1]]):
        line = ax.errorbar(
            eff[:, 0],
            eff[:, 1] * factor * 100,
            yerr=eff[:, 2],
            color=cmaps[i](0.5),
            ls="solid" if k == 0 else "dashed",
            marker="none",
            # ls="none",
            # markersize=3,
            capsize=0,
            label=labels[i] if k == 0 else None,
        )
    if i == 0 and line:
        ax.annotate(
            rf"(in)elastic $\times$ {elfactor}",
            xy=(22.5, 20),
            **sty.ann,
            color=line[0].get_color(),
        )
    # ax.fill_between(lis[-1][:, 0], y1=lis[0][:, 1], y2=lis[-1][:, 1])


# Deuton
ax: mplaxes.Axes = axs[1]
for i, lis in enumerate(effs[2:]):
    factor = 1 if i != 0 else elfactor
    line = None
    for k, eff in enumerate([lis[0], lis[-1]]):
        line = ax.errorbar(
            eff[:, 0],
            eff[:, 1] * factor * 100,
            yerr=eff[:, 2],
            color=cmaps[i + 2](0.5),
            ls="solid" if k == 0 else "dashed",
            marker="none",
            # ls="none",
            # markersize=3,
            capsize=0,
            label=labels[i + 2] if k == 0 else None,
        )
    if i == 0 and line:
        ax.annotate(
            rf"(in)elastic $\times$ {elfactor}",
            xy=(20, 20),
            **sty.ann,
            color=line[0].get_color(),
        )

## Common settings
for ax in axs:
    ax.legend()
    ax.set_xlim(0, 30)
    ax.set_ylim(0)
    ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
    ax.set_ylabel(r"$\varepsilon$ [%]")

phys.utils.annotate_subplots(axs)

fig.tight_layout()
fig.savefig(sty.thesis + "e796_effs.pdf", dpi=300)
plt.show()
