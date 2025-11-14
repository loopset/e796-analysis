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
import uncertainties.unumpy as unp

import sys

sys.path.append("../")
import styling as sty

# Read fits
labels = ["(p,p)", "(d,d)"]
fits = []
for label in labels:
    fit = np.load(
        f"../analysis/Outputs/sigmas_fit_{label}.npy", allow_pickle=True
    ).item()
    fits.append(fit)

# Experimental free sigma (p,p)
pp = FitInterface("../../Fits/pp/Outputs/fit_free_sigma.root")
dd = FitInterface("../../Fits/dd/Outputs/fit_free_sigma.root")
frees = [pp, dd]


fig, ax = plt.subplots()
ls = []
for fit in fits:
    x, y = fit.linspace()
    ls.append(ax.plot(x, y))

for i, free in enumerate(frees):
    xfree = list(free.fEx.values())
    yfree = list(free.fSigmas.values())
    ax.errorbar(
        unp.nominal_values(xfree),
        unp.nominal_values(yfree),
        yerr=unp.std_devs(yfree),
        **sty.errorbar,
        color=ls[i][0].get_color(),
    )

ax.axvline(x=phys.Particle("20O").get_sn(), color="crimson", ls="--", label="S2n")
ax.set_ylim(0)
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel(r"$\sigma$ [MeV]")
ax.legend()

fig.tight_layout()
fig.savefig(sty.thesis + "exp_sigmas_pp_dd.pdf", dpi=300)
plt.show()
