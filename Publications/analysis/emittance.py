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

file = "./Inputs/emittance.root"
up = uproot.open(file)
if not file:
    raise ValueError
hxy = up["hYthetaXY"].to_hist() #type: ignore
hxz = up["hYthetaXZ"].to_hist() #type: ignore

overflow = 1500
fig, axs = plt.subplots(1, 2, figsize=(9, 3.5))
ax: mplaxes.Axes = axs[0]
phys.utils.set_hist_overflow(hxy, overflow)
hxy.plot(ax=ax, cmax=overflow, cbar=False, **sty.base2d)
ax.set_ylabel(r"$\theta_{XY}$ [$\circ$]")
ax.xaxis.set_major_locator(mpltick.MaxNLocator(nbins=3, integer=True))

ax=axs[1]
phys.utils.set_hist_overflow(hxz, overflow)
ret = hxz.plot(ax=ax, cmax=overflow, cbar=True, **sty.base2d)
ax.set_ylabel(r"$\theta_{XZ}$ [$\circ$]")
ret[1].locator = mpltick.MaxNLocator(nbins=5)
ticks = ret[1].get_ticks()
ret[1].set_ticks(ticks)

phys.utils.annotate_subplots(axs)

# Common axis settings
for ax in axs:
    ax.set_xlim(105, 155)
    ax.set_ylim(-4 ,4)
    ax.set_xlabel("")

fig.tight_layout()
fig.supxlabel(r"Y [mm]", x=0.5, y=0.045, fontsize=plt.rcParams["axes.labelsize"], ha="center", va="center")
fig.subplots_adjust(bottom=0.145)
fig.savefig(sty.thesis + "emittance.pdf", dpi=300)
plt.show()