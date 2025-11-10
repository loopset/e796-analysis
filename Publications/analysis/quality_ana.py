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
energy = 700

# Init histograms
hkins = []
for i in range(2):
    h = (
        hist.Hist.new.Reg(200, 0, 90, label=r"$\theta_{lab}$ [$\circ$]")
        .Reg(200, 0, 15, label=r"E$_{lab}$ [MeV]")
        .Double()
    )
    hkins.append(h)

# Fill histograms
for i, (target, light) in enumerate(pairs):
    side = "_side" if target == light else "_front"
    data = uproot.open(f"../../PostAnalysis/RootFiles/Pipe3/tree_20O_{target}_{light}{side}_juan_RPx.root:Sel_Tree").arrays(["EVertex", "fThetaLight"])  # type: ignore
    if target == "1H":
        idx = 0
    else:
        idx = 1
    hkins[idx].fill(data["fThetaLight"], data["EVertex"])

# Kinematics plots
fig, axs = plt.subplots(1, 2, figsize=(10, 4))
overflow = max(hkins[0].values().max(), hkins[1].values().max())

for i, h in enumerate(hkins):
    ax: mplaxes.Axes = axs[i]
    phys.utils.set_hist_overflow(hkins[i], overflow)
    ret = hkins[i].plot(ax=ax, cbar=False if i < 1 else True, norm=mpcolor.LogNorm(vmax=overflow), **sty.base2d_nocmin)
    # Plot theo kin
    channels = labels[:2] if i == 0 else labels[2:]
    for channel in channels:
        kin = phys.Kinematics(f"20O{channel}@700").get_line3()
        ax.plot(kin[0], kin[1], label=channel)
    # Legend
    ax.legend()
    #Axis settings
    ax.locator_params(axis="both", nbins=4)
    # Cbar
    if i == 1:
        cbar = ret[1]
        phys.utils.apply_ROOT_colorbar(cbar)


phys.utils.annotate_subplots(axs)

fig.tight_layout()
fig.savefig(sty.thesis + "ana_kins.pdf", dpi=300)
plt.show()
