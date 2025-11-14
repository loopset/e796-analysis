from typing import Dict
import pyphysics as phys
from pyphysics.actroot_interface import FitInterface
import hist
import uproot

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mpcolor
import matplotlib.lines as mplines
import uncertainties as un

import sys

sys.path.append("../")
import styling as sty

# Read data
files = [
    "../../PostAnalysis/RootFiles/Pipe3/tree_20O_1H_1H_side_juan_RPx.root:Sel_Tree",
    "../../PostAnalysis/RootFiles/Pipe3/tree_20O_2H_2H_side_juan_RPx.root:Sel_Tree",
]
labels = ["(p,p)", "(d,d)"]
hs = []
for file in files:
    data = uproot.open(file).arrays(["EVertex", "fThetaLight"])  # type: ignore
    h = (
        hist.Hist.new.Reg(400, 0, 120, label=r"$\theta_{lab}$ [$\circ$]")
        .Reg(600, 0, 15, label=r"$E_{lab}$ [MeV]")
        .Double()
    )
    h.fill(data["fThetaLight"], data["EVertex"])
    hs.append(h)

## States to draw
exs = [0, 1.68, 4.1, 5.6]

# Figure
fig, axs = plt.subplots(1, 2, figsize=(9, 4.5))
for i, ax in enumerate(axs):
    ax: mplaxes.Axes = axs[i]
    hs[i].plot(ax=ax, **sty.base2d)
    # (p,p)
    if i == 0:
        ax.set_xlim(40, 110)
        ax.set_ylim(2, 8)
        for ex in exs[:2]:
            theo = phys.Kinematics(f"20O(p,p)@700|{ex}").get_line3()
            ax.plot(theo[0], theo[1])
    else:  ## (d,d)
        ax.set_xlim(50, 100)
        ax.set_ylim(2, 12)
        for ex in exs:
            theo = phys.Kinematics(f"20O(d,d)@700|{ex}").get_line3()
            ax.plot(theo[0], theo[1])

    ## Annotations
    ax.annotate(
        rf"$^{{20}}$O{labels[i]}",
        xy=(0.8, 0.9),
        xycoords="axes fraction",
        fontweight="bold",
        **sty.ann,
    )

# Common axis settings
for ax in axs:
    ax.set_xlabel("")
    ax.set_ylabel("")

# Annotate axis
phys.utils.annotate_subplots(axs)
# X label
fig.supxlabel(
    r"$\theta_{lab}$ [$\circ$]", x=0.525, fontsize=plt.rcParams["axes.labelsize"]
)
# Y label
fig.supylabel(r"$E_{lab}$ [MeV]", fontsize=plt.rcParams["axes.labelsize"])

fig.tight_layout()
fig.subplots_adjust(bottom=0.12)

fig.savefig(sty.thesis + "20O_pp_dd_kins.pdf", dpi=300)
plt.show()
