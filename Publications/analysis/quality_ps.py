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
labels = ["d-breakup", "20O 1n as pd", "20O 1n", "19O 1n"]
files = [
    "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_2H_2H_0.00_nPS_-1_pPS_0.root",
    "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_2H_2H_0.00_nPS_-2_pPS_0.root",
    "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_2H_2H_0.00_nPS_1_pPS_0.root",
    "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_2H_3H_0.00_nPS_1_pPS_0.root",
]
channels = [
    "(p,p)",
    "(p,d)",
    "(d,d)",
    "(d,t)",
    # f"(d,d)@700|{phys.Particle('20O').get_sn()}",
    # f"(d,t)@700|{phys.Particle('19O').get_sn()}",
]

sns = [None, None, phys.Particle("20O").get_sn(), phys.Particle("19O").get_sn()]
sns_labels = [None, None, "^{20}O", "^{19}O"]

# Init histograms
hkins = []
for i in range(2):
    h = (
        hist.Hist.new.Reg(200, 0, 90, label=r"$\theta_{lab}$ [$\circ$]")
        .Reg(200, 0, 15, label=r"E$_{lab}$ [MeV]")
        .Double()
    )
    hkins.append(h)


def fill_hist_ps(h: hist.BaseHist, file: str) -> None:
    data = uproot.open(file + ":SimulationTTree").arrays(["EVertex", "theta3Lab", "weight"])  # type: ignore
    haux = h.copy().reset()
    haux.fill(data["theta3Lab"], data["EVertex"], weight=data["weight"])
    norm = 1e5
    factor = haux.sum() / norm  # type:ignore
    haux /= factor

    ## And add to hist
    h += haux


# Proton
fill_hist_ps(hkins[0], files[0])
fill_hist_ps(hkins[0], files[1])
# Deutons
fill_hist_ps(hkins[1], files[2])
fill_hist_ps(hkins[1], files[3])

# Kinematics plots
fig, axs = plt.subplots(1, 2, figsize=(10, 4))
overflow = max(hkins[0].values().max(), hkins[1].values().max())

for i, h in enumerate(hkins):
    ax: mplaxes.Axes = axs[i]
    phys.utils.set_hist_overflow(hkins[i], overflow)
    ret = hkins[i].plot(
        ax=ax,
        cbar=False if i < 1 else True,
        norm=mpcolor.LogNorm(vmin=1, vmax=overflow),
        **sty.base2d_nocmin,
    )
    # Plot theo kin
    chs = channels[:2] if i == 0 else channels[2:]
    sn = sns[:2] if i == 0 else sns[2:]
    lsn = sns_labels[:2] if i == 0 else sns_labels[2:]
    for j, c in enumerate(chs):
        if not len(c):
            continue
        kin = phys.Kinematics(f"20O{c}@700").get_line3()
        l = ax.plot(kin[0], kin[1], label=c)
        # Plot Sn if provided
        if sn[j]:
            aux = phys.Kinematics(f"20O{c}@700|{sn[j]}").get_line3()
            ax.plot(
                aux[0],
                aux[1],
                ls="--",
                color=l[0].get_color(),
                label=rf"$E_{{x}}=S_{{n}}({lsn[j]})$",
            )
    # Legend
    if i == 0:
        ax.legend()
    else:
        ax.legend(
            loc="lower left", ncols=2, fontsize=12, columnspacing=0.5, handlelength=1
        )
    # Axis settings
    ax.locator_params(axis="both", nbins=4)
    # Cbar
    if i == 1:
        cbar = ret[1]
        phys.utils.apply_ROOT_colorbar(cbar)

# Annotations in first
ax = axs[0]
ax.annotate(
    r"d break-up",
    xy=(64, 5.4),
    xytext=(57, 9),
    **sty.ann,
    arrowprops=sty.arrowprops,
)
ax.annotate(
    r"$^{20}O \rightarrow ^{19}O + n$",
    xy=(30, 10),
    xytext=(57, 12.5),
    **sty.ann,
    arrowprops=sty.arrowprops,
)

phys.utils.annotate_subplots(axs, x=0.9)

fig.tight_layout()
fig.savefig(sty.thesis + "simu_ps.pdf", dpi=300)
plt.show()
