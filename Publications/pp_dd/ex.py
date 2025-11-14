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
import copy

import sys

sys.path.append("../")
import styling as sty

# Read data
files = [
    "../../Fits/pp/Outputs/fit_juan_RPx.root",
    "../../Fits/dd/Outputs/fit_juan_RPx.root",
]
labels = ["(p,p)", "(d,d)"]
fits = [FitInterface(f) for f in files]

# For dd, clone to plot g.s and 1st separately
clones = [copy.deepcopy(fits[-1]) for i in range(2)]
scales = [0.5, 1]
limits = [1, 2.95]
for i, clone in enumerate(clones):
    low = limits[i] if 0 <= i < len(limits) else -np.inf
    up = limits[i + 1] if 0 <= i + 1 < len(limits) else +np.inf
    clone.scale_limit(scales[i], xrange=(low, up))
# Ground-state scaling
scalegs = 0.075
fits[-1].scale_limit(scalegs, (-np.inf, 1))

fig, axs = plt.subplots(2, 1, figsize=(9, 6), layout="constrained")
for i, fit in enumerate(fits):
    ax: mplaxes.Axes = axs.flat[i]
    plt.sca(ax)
    # Hist
    fit.plot_hist(**sty.styles["ex"])
    fit.format_ax(ax)
    # Global
    fit.plot_global(**sty.styles["global"])
    # Individual
    for key in fit.fFuncs.keys():
        fit.plot_func(key)

    # Individual axis settings
    if i == 0:
        ax.set_yscale("log")
        fit.format_ax(ax)
        ax.set_ylim(2)
        ax.set_xlim(-10)
        ax.set_xlabel("")
        ax.xaxis.set_major_locator(mpltick.MaxNLocator(integer=True))
    else:
        for clone in clones:
            clone.plot_hist(**sty.styles["ex"])
            clone.plot_global(**sty.styles["global"])
            for key in clone.fFuncs.keys():
                clone.plot_func(key)
        ax.set_xlim(-2.5, 15)
    # Annotations
    ax.annotate(
        rf"$^{{20}}$O{labels[i]}",
        xy=(0.9, 0.8),
        xycoords="axes fraction",
        fontweight="bold",
        **sty.ann,
    )

ax = axs.flat[-1]
ax.annotate(rf"g.s. $\times$ {scalegs}", xy=(-1.2, 230), **sty.ann)
ax.annotate(rf"$1^{{st}}\times$ {scales[0]}", xy=(2.7, 230), **sty.ann)

# Annotations
phys.utils.annotate_subplots(axs, x=0.9, y=0.9)
fig.savefig(sty.thesis + "20O_pp_dd_exs.pdf", dpi=300)
plt.show()
