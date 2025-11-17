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

# Limits
xlim = (-0.5, 7)

fig, ax = plt.subplots(figsize=(4.5, 3.5))
ls = []
for fit in fits:
    fit: np.polynomial.Polynomial
    fit.domain = xlim  # type:ignore
    x, y = fit.linspace()
    ls.append(ax.plot(x, y, ls="--"))

for i, free in enumerate(frees):
    xfree = np.array(list(free.fEx.values()))
    yfree = np.array(list(free.fSigmas.values()))
    mask = unp.nominal_values(xfree) < 6.2
    ax.errorbar(
        unp.nominal_values(xfree[mask]),
        unp.nominal_values(yfree[mask]),
        yerr=unp.std_devs(yfree[mask]),
        **sty.errorbar,
        color=ls[i][0].get_color(),
        label=r"$^{20}$O" + labels[i],
    )

ax.set_xlim(*xlim)
ax.set_ylim(0, 0.5)
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel(r"$\sigma$ [MeV]")
ax.legend(loc="lower left", ncols=2)

fig.tight_layout()
fig.savefig(sty.thesis + "exp_sigmas_pp_dd.pdf", dpi=300)
plt.show()
