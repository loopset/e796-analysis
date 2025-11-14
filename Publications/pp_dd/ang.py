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

import sys

sys.path.append("../")
import styling as sty

sfs = SFInterface("../../Fits/pp/Outputs/sfs.root")

states = ["g0", "g1"]
labels = ["g.s\n" + r"$0^+$", "1.63 MeV\n" + r"$2^+$"]
fig, axs = plt.subplots(1, 2, figsize=(8, 4), layout="constrained")
for i, ax in enumerate(axs):
    ax: mplaxes.Axes
    ax.set_yscale("log")
    sfs.plot_exp(states[i], ax=ax)
    ax.set_prop_cycle(sty.cyclers["l012"])
    sfs.plot_models(states[i], ax=ax)
    sfs.format_ax(states[i], ax)
    ax.set_xlabel("")
    ax.annotate(labels[i], xy=(0.85, 0.85), xycoords="axes fraction", **sty.ann)

# Axis settings
ax = axs[0]
ax.set_ylim(ax.get_ylim()[0] * 1.2, ax.get_ylim()[1] * 1.9)
ax.legend()

ax=axs[-1]
ax.set_ylim(ax.get_ylim()[0] * 1.2, ax.get_ylim()[1] * 1.9)
phys.utils.apply_log_formatter(ax)
ax.set_ylabel("")
ax.yaxis.tick_right()
ax.yaxis.set_ticks_position("both")

# X label
fig.supxlabel(r"$\theta_{CM}$ [$\circ$]", x=0.525, fontsize=plt.rcParams["axes.labelsize"])

phys.utils.annotate_subplots(axs)
# fig.tight_layout()
fig.savefig(sty.thesis + "20O_pp_ang.pdf", dpi=300)

plt.show()
