import pyphysics as phys
from pyphysics.actroot_interface import FitInterface, SFInterface
import hist
import uproot

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mpcolor
import matplotlib.lines as mplines
import uncertainties as un
import pandas as pd

import sys

sys.path.append("../")
import styling as sty

dd = SFInterface("../../Fits/dd/Outputs/sfs.root")
rebin = SFInterface("../../Fits/dd/Outputs/rebin_sfs.root")
states = ["g1", "g2", "g3", "g4", "g5", "g6"]
isRebin = [False, False, False, True, True, True]

nrow = 3
ncol = 2

fig, axs = plt.subplots(2, 3, figsize=(10, 4.5), sharex=True, layout="constrained")
for i, state in enumerate(states):
    ax: mplaxes.Axes = axs.flat[i]
    face = rebin if isRebin[i] else dd
    face.plot_exp(state, ax=ax)
    face.plot_models(state, ax=ax)
    face.format_ax(state, ax)
    ax.legend(ncols=2)

for ax in axs.flat:
    ax.set_ylabel("")
for ax in axs.flat:
    ax.set_xlabel("")
# Manual labels
axs[1, 1].set_xlabel(r"$\theta_{CM}$ [$\circ$]")
fig.supylabel(
    r"$d\sigma/d\Omega$ [$mb\cdot sr^{-1}$]", fontsize=plt.rcParams["axes.labelsize"]
)

# fig.tight_layout()
fig.savefig(sty.thesis + "20O_dd_ine.pdf", dpi=300)
plt.show()
