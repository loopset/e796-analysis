import copy
from turtle import right

import hist.plot
from matplotlib import hatch
import pyphysics as phys
import numpy as np
from pyphysics.actroot_interface import FitInterface
import hist
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mplcolor
from matplotlib.patches import Patch
import matplotlib.pyplot as plt
import sys
from matplotlib.legend_handler import HandlerBase
from matplotlib.patches import Rectangle
import uproot

sys.path.append("../")

import styling as sty

exp = uproot.open(
    "../../PostAnalysis/RootFiles/Pipe3/tree_20O_1H_2H_front_juan_RPx.root:Sel_Tree"
).arrays(  # type: ignore
    ["EVertex", "fThetaLight"]
)  # type: ignore

# Histogram
xmodel = (200, 0, 60)
ymodel = (200, 0, 20)
h = (
    hist.Hist.new.Reg(*xmodel, label=r"$\theta_{lab}$ [$\circ$]")
    .Reg(*ymodel, label=r"$E_{lab}$ [MeV]")
    .Double()
)
h.fill(exp["fThetaLight"], exp["EVertex"])


# Fit interface
fit = FitInterface("../../Fits/pd/Outputs/fit_juan_RPx.root")
gsfactor = 1

# Declare background peaks
back = []
ignored = []

fig, axs = plt.subplots(
    1,
    2,
    figsize=(8.5, 4),
)

# Kin
ax: mplaxes.Axes = axs[0]
h.plot(ax=ax, **sty.base2d)
## States to draw
exs = [0, 1.4, 3.1]
for ex in exs:
    theo = phys.Kinematics(f"20O(p,d)@700|{ex}").get_line3()
    label = f"{ex:.1f}" if ex > 0 else "g.s"
    ax.plot(theo[0], theo[1], label=label)
# Legend
ax.legend(title=r"$E_{x}$ / MeV", title_fontsize=12)
## Annotations
ax.annotate(
    rf"$^{{20}}$O(p,d)",
    xy=(0.15, 0.9),
    xycoords="axes fraction",
    fontweight="bold",
    **sty.ann,
)


# Fit
ax: mplaxes.Axes = axs[-1]
plt.sca(ax)
for i, inter in enumerate([fit]):
    inter.plot_hist(ax=ax, **sty.styles["ex"], label="Exp." if i == 0 else None)
    inter.plot_global(**sty.styles["global"], label="Fit" if i == 0 else None)
    withCont = False
    withIgnored = False
    for j, key in enumerate(inter.fFuncs.keys()):
        opts = {}
        if key in back:
            opts.update(sty.styles["ps"])
            opts.update(
                dict(
                    ls="-",
                    ec="grey",
                    fc="grey",
                    fill=True,
                    alpha=0.5,
                    label="(p,d) cont." if not withCont else None,
                ),
            )
            withCont = True
        elif key in ignored:
            opts.update(sty.styles["ps"])
            opts.update(
                dict(
                    ls="-",
                    ec="grey",
                    fc="grey",
                    # fill=True,
                    hatch="xxx",
                    alpha=0.5,
                    label="Not studied" if not withIgnored else None,
                )
            )
            withIgnored = True
        elif key == "ps0":
            opts.update(sty.styles["ps"])
            opts.update(
                dict(ls="-", ec="purple", fc="purple", fill=True, alpha=0.1, zorder=1),
                label="(d,d) 1n PS" if i == 0 else None,
            )
        elif key == "v7":
            opts.update(dict(hatch="..."))
        elif i == 0 and j == 0:
            opts.update(dict(label="States"))
        inter.plot_func(key, **opts)
    if i == 0:
        inter.format_ax(ax)


# Format axes
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_xlim(-7.5, 5)


handles, labels = ax.get_legend_handles_labels()
ax.legend(
    handles=[handles[-1]] + handles[:-1],
    labels=[labels[-1]] + labels[:-1],
    # loc="upper right",
    labelspacing=0.4,
    # borderaxespad=0.3,
    # bbox_to_anchor=(1.02, 1),
)

fig.tight_layout()
fig.savefig(sty.thesis + "20O_pd_kin_ex.pdf", dpi=300)
plt.show()
