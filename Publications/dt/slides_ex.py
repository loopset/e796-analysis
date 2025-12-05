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

sys.path.append("../")

import styling as sty


# Fit interface
fit = FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
ori = copy.deepcopy(fit)
gsbreak = 1.0
gsfactor = 0.25
clone = copy.deepcopy(fit)
clone.scale_limit(scale=gsfactor, xrange=(-np.inf, gsbreak))
fit.scale_limit(scale=1, xrange=(gsbreak, np.inf))

# Declare background peaks
back = ["v8", "v9", "v10"]
ignored = ["v11", "v12"]

# Draw — two vertical subplots, lower one shorter
fig, ax = plt.subplots(
    1,
    1,
    figsize=(10.5, 4),
    constrained_layout=True,
)
ax: mplaxes.Axes
plt.sca(ax)
for i, inter in enumerate([clone, fit]):
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
                label="1n PS" if i == 0 else None,
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
ax.set_xlim(-2.5)

# Sn
o19 = phys.Particle("19O")
ax.axvline(o19.get_sn(), color="purple", **sty.styles["sn"], ls="dotted")
ax.annotate(
    rf"S$_{{\mathrm{{n}}}} =$ {o19.get_sn():.2f} MeV",
    xy=(o19.get_sn() + 2.5, 875 * gsfactor),
    **sty.ann,
)
ax.axvline(o19.get_s2n(), color="hotpink", **sty.styles["sn"], ls="dashdot")
ax.annotate(
    rf"S$_{{\mathrm{{2n}}}} =$ {o19.get_s2n():.2f} MeV",
    xy=(o19.get_s2n() + 2.5, 875 * gsfactor),
    **sty.ann,
)

# Annotations
ax.annotate(rf"g.s $\times$ {gsfactor:.2f}", xy=(1.4, 180), **sty.ann)

isospin = Patch(facecolor="none", edgecolor="C3", hatch="...", label="T = 5/2")
handles, labels = ax.get_legend_handles_labels()
ax.legend(
    handles=[handles[-1]] + handles[:-1] + [isospin],
    labels=[labels[-1]] + labels[:-1] + [isospin.get_label()],
    loc="upper right",
    labelspacing=0.4,
    # borderaxespad=0.3,
    # bbox_to_anchor=(1.02, 1),
)
fig.savefig("/media/Data/Docs/SSW/figures/ex.png", dpi=600)
plt.show()
