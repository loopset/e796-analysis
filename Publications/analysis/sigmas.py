from boost_histogram import loc
import pyphysics as phys
import awkward as ak
from pyphysics.root_interface import parse_tf1, parse_th1, parse_tgraph
import hist
import uproot
import re

import numpy as np
from numpy.polynomial import Polynomial
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

gs = []
for target, light in pairs:
    file = r.TFile(f"../../Simulation/Outputs/juan_RPx/sigmas_20O_{target}_{light}.root")  # type: ignore
    g = parse_tgraph(file.Get("gsigmas"))
    gs.append(g)

cmaps = [
    plt.colormaps.get_cmap("Blues"),
    plt.colormaps.get_cmap("Reds"),
    plt.colormaps.get_cmap("Oranges"),
    plt.colormaps.get_cmap("Greens"),
]

# Plot
fig, axs = plt.subplots(1, 2, figsize=(10, 4))
for i, g in enumerate(gs):
    ax: mplaxes.Axes = axs[0] if i <= 1 else axs[1]
    err = ax.errorbar(
        g[:, 0],
        g[:, 1],
        yerr=g[:, 2],
        label=labels[i],
        color=cmaps[i](0.5),
        marker="o",
        ls="none",
        # markersize=3,
    )
    # Fit
    fit = phys.fit_poln(g[:, 0], g[:, 1], n=1)
    x = np.linspace(0, ax.get_xlim()[1], 5)
    y = np.polyval(unp.nominal_values(fit), x)
    ax.plot(x, y, ls="--", color=cmaps[i](0.5))
    # Legend
    ax.legend(loc="center left")
    # Format
    ax.set_xlabel(r"E$_{x}$ [MeV]")
    ax.set_ylabel(r"$\sigma$ [MeV]")
    # Write sigma file to outputs
    p = Polynomial(
        coef=unp.nominal_values(fit[::-1]),
        domain=(x.min(), x.max()),
        window=(x.min(), x.max()),
    )
    np.save(f"./Outputs/sigmas_fit_{labels[i]}.npy", p)  # type: ignore

phys.utils.annotate_subplots(axs, x=0.9)

fig.tight_layout()
fig.savefig(sty.thesis + "simu_sigmas.pdf", dpi=300)
plt.show()
