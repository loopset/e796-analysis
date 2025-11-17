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

pp = SFInterface("../../Fits/pp/Outputs/sfs.root")
dd = SFInterface("../../Fits/dd/Outputs/sfs.root")
sfs = [pp, dd]

colors = [
    plt.colormaps.get("Blues")(np.linspace(0.5, 1, 3)),  # type: ignore
    plt.colormaps.get("Oranges")(np.linspace(0.5, 1, 3)),  # type: ignore
]
ls = [l for l in sty.ls.values()]

state = "g0"
labels = ["(p,p)", "(d,d)"]
fig, axs = plt.subplots(1, 2, figsize=(8, 4), layout="constrained")
for i, ax in enumerate(axs):
    ax: mplaxes.Axes
    sf = sfs[i]
    cycler = sty.cycler.cycler(color=colors[i]) + sty.cycler.cycler(ls=ls)
    ax.set_yscale("log")
    sf.plot_exp(state, ax=ax)
    # ax.set_prop_cycle(sty.cyclers["l012"])
    ax.set_prop_cycle(cycler)
    sf.plot_models(state, ax=ax)
    sf.format_ax(state, ax)
    ax.set_xlabel("")
    ax.annotate(
        r"$^{20}$O" + labels[i] + "\ng.s",
        xy=(0.85, 0.825),
        xycoords="axes fraction",
        **sty.ann,
        fontweight="bold"
    )

# Axis settings
ax = axs[0]
ax.set_ylim(ax.get_ylim()[0] * 1.2, ax.get_ylim()[1] * 1.9)
ax.legend()

ax = axs[-1]
ax.set_ylim(ax.get_ylim()[0] * 1.1, ax.get_ylim()[1] * 4)
phys.utils.apply_log_formatter(ax)
ax.set_ylabel("")
ax.yaxis.tick_right()
ax.yaxis.set_ticks_position("both")
ax.legend()

# X label
fig.supxlabel(
    r"$\theta_{CM}$ [$\circ$]", x=0.525, fontsize=plt.rcParams["axes.labelsize"]
)

phys.utils.annotate_subplots(axs, x=0.85)
# fig.tight_layout()
fig.savefig(sty.thesis + "20O_gs_ang.pdf", dpi=300)

plt.show()
