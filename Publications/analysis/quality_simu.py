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

    pattern = re.compile(rf"tree_20O_{target}_{light}_(\d+.\d+)_nPS_0_pPS_0\.root$")
    path = f"/media/Data/E796v2/Simulation/Outputs/juan_RPx/"
    return [f"{path}/{f}:SimulationTTree" for f in os.listdir(path) if pattern.match(f)]


# Init histograms
hkins = []
for i in range(2):
    h = (
        hist.Hist.new.Reg(300, 0, 90, label=r"$\theta_{lab}$ [$\circ$]")
        .Reg(300, 0, 15, label=r"E$_{lab}$ [MeV]")
        .Double()
    )
    hkins.append(h)

# Fill histograms
for i, (target, light) in enumerate(pairs):
    files = get_simu_files(target, light)
    idx = 0 if target == "1H" else 1
    for file in files:
        data = uproot.open(file).arrays(["EVertex", "theta3Lab"])  # type: ignore
        h = hkins[0].copy().reset()
        h.fill(data["theta3Lab"], data["EVertex"])
        norm = 1e5
        factor = h.sum() / norm
        h /= factor
        hkins[idx] += h

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
    channels = labels[:2] if i == 0 else labels[2:]
    for channel in channels:
        kin = phys.Kinematics(f"20O{channel}@700").get_line3()
        ax.plot(kin[0], kin[1], label=channel)
    # Legend
    ax.legend()
    # Axis settings
    ax.locator_params(axis="both", nbins=4)
    # Cbar
    if i == 1:
        cbar = ret[1]
        phys.utils.apply_ROOT_colorbar(cbar)


phys.utils.annotate_subplots(axs)

fig.tight_layout()
fig.savefig(sty.thesis + "simu_kins.pdf", dpi=300)
plt.show()
