from typing import List
import pyphysics as phys
from pyphysics.root_interface import parse_tgraph, parse_tf1, parse_th1
import hist
import uproot

import re
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mplcolor

import ROOT as r

import sys

sys.path.append("../")
import styling as sty


def get_function(file, hname, fname):
    h = file.Get(hname)
    return parse_tf1(h.GetFunction(fname))


# Front file
upfront = uproot.open("./Inputs/angle_front.root")
front = r.TFile("./Inputs/angle_front.root")  # type: ignore
hfront = upfront["hExRPLeg"].to_hist()  # type: ignore
hcorr1 = upfront["hCorr1"].to_hist()  # type: ignore
fcorr1 = get_function(front, "hCorr1", "fitth2")
hcorr2 = upfront["hCorr2"].to_hist()  # type: ignore
fcorr2 = get_function(front, "hCorr2", "fitth2")
hfrontok = upfront["hExRPOK"].to_hist()  # type: ignore

# Side file
upside = uproot.open("./Inputs/angle_side.root")
hside = upside["hExRPLeg"].to_hist()  # type: ignore

## Figure 1: introduce the problem
fig, axs = plt.subplots(1, 2, figsize=(9, 3.5))
ax: mplaxes.Axes = axs[0]
hside.plot(ax=ax, **sty.base2d)
ax.axhspan(-1, 0.82, color="plum", alpha=0.25)


ax: mplaxes.Axes = axs[1]
hfront.plot(ax=ax, **sty.base2d)
ax.axhspan(-2.19, 0.64, color="plum", alpha=0.25)

## Common axis settings
for ax in axs:
    ax.set_xlabel("")
    ax.set_ylabel("")
    ax.set_ylim(-5, 5)

## Annotations
labels = ["(p,p)", "(p,d)"]
for ax, label in zip(axs, labels):
    ax.annotate(
        label, xy=(0.85, 0.9), xycoords="axes fraction", **sty.ann, fontweight="bold"
    )
phys.utils.annotate_subplots(axs)

# Titles
fig.supxlabel(
    "RP.X [mm]",
    x=0.525,
    y=0.04,
    ha="center",
    va="center",
    fontsize=plt.rcParams["axes.labelsize"],
)
fig.supylabel(r"E$_{x}$ [MeV]", x=0.005, fontsize=plt.rcParams["axes.labelsize"])
fig.tight_layout()
fig.subplots_adjust(left=0.065, bottom=0.125)

## Figure 2: Study dependences for (p,d)
plt.close("all")
fig = plt.figure(figsize=(9, 4.5))
grid = fig.add_gridspec(nrows=2, ncols=2, width_ratios=[1, 1])

# Some particular settings for these plots
axlabelsize = 14
cbar = False
axlabelpad = 0
# Correction 1
ax: mplaxes.Axes = fig.add_subplot(grid[0, 0])
hcorr1.plot(ax=ax, cbar=cbar, **sty.base2d)
ax.plot(fcorr1[:, 0], fcorr1[:, 1], color="darkorange", label=r"2$^{nd}$ order fit")
ax.legend()
ax.set_xlabel("RP.X [mm]", fontsize=axlabelsize, labelpad=axlabelpad)
ax.set_ylabel(
    r"$\delta \theta_{1}$ $[\circ]$", fontsize=axlabelsize, labelpad=axlabelpad
)

# Correction 2
ax: mplaxes.Axes = fig.add_subplot(grid[1, 0])
hcorr2[:, ::2j].plot(ax=ax, cbar=cbar, **sty.base2d)
ax.plot(fcorr2[:, 0], fcorr2[:, 1], color="darkorange", label=r"1$^{st}$ order fit")
ax.legend()
ax.set_ylabel(
    r"$\delta \theta_{2}$ $[\circ]$", fontsize=axlabelsize, labelpad=axlabelpad
)
ax.set_xlabel(r"$\theta_{1}$ $[\circ]$", fontsize=axlabelsize, labelpad=axlabelpad)
ax.set_xlim(0, 40)

# Final result
ax: mplaxes.Axes = fig.add_subplot(grid[:, 1])
hfrontok.plot(ax=ax, cbar=cbar, **sty.base2d)
ax.set_xlabel("RP.X [mm]", fontsize=axlabelsize, labelpad=axlabelpad)
ax.set_ylabel(r"$E_{x}$ [MeV]", fontsize=axlabelsize, labelpad=axlabelpad)
ax.set_ylim(-5, 5)
ax.axhline(0, color="crimson", ls="--", lw=1.5)
ax.annotate(
    "(p,d)", xy=(0.9, 0.85), xycoords="axes fraction", **sty.ann, fontweight="bold"
)

phys.utils.annotate_subplots(fig.axes, y=0.85)
fig.axes[-1].texts[-1].set_position((0.9, 0.925))
fig.tight_layout()

## Figure 3: Changes in Ex histograms


plt.show()
