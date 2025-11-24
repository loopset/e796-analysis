import copy
from turtle import right

import hist.plot
import pyphysics as phys
import numpy as np
from pyphysics.actroot_interface import FitInterface
import hist
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mplcolor
import matplotlib.pyplot as plt
import sys

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
back = ["v8", "v9", "v10", "v11"]

# Draw — two vertical subplots, lower one shorter
fig, (ax, ax_pull) = plt.subplots(
    2,
    1,
    figsize=(10, 5),
    gridspec_kw={"height_ratios": [4, 1]},
    sharex=True,
    constrained_layout=True,
)
ax: mplaxes.Axes
ax_pull: mplaxes.Axes
plt.sca(ax)
for i, inter in enumerate([clone, fit]):
    inter.plot_hist(ax=ax, **sty.styles["ex"], label="Exp." if i == 0 else None)
    inter.plot_global(**sty.styles["global"], label="Fit" if i == 0 else None)
    withCont = False
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
        elif key == "ps0":
            opts.update(sty.styles["ps"])
            opts.update(
                dict(ls="-", ec="purple", fc="purple", fill=True, alpha=0.1, zorder=1),
                label="1n PS" if i == 0 else None,
            )
        elif i == 0 and j == 0:
            opts.update(dict(label="States"))
        inter.plot_func(key, **opts)
    if i == 0:
        inter.format_ax(ax)

## Build pull array
pulls = []
for i, h in enumerate([clone, fit]):  # very important order to concatenate later
    if h.fGlobal is not None and h.fHistEx is not None:
        spline = phys.create_spline3(h.fGlobal[:, 0], h.fGlobal[:, 1])
        aux = []
        for b, x in enumerate(h.fHistEx.axes[0].centers):
            yhist = h.fHistEx[b]
            uhist = np.sqrt(yhist)  # type: ignore
            yfit = spline(x)
            if yhist < 1 or uhist == 0:  # type: ignore
                pull = 0
            else:
                pull = (yhist - yfit) / uhist  # type:ignore
            aux.append(pull)
        pulls.append(np.array(aux))
# Merge both pulls from two regions
pulls = np.concatenate((pulls[0], pulls[1]))
hist.plot.plot_pull_array(
    ori.fHistEx,  # type: ignore
    np.array(pulls),
    ax=ax_pull,
    bar_kwargs=dict(color="crimson", alpha=0.5),
    pp_kwargs=dict(num=3, color="none"),
)
ax_pull.axhspan(ymin=-1, ymax=1, color="crimson", alpha=0.15)
ax_pull.set_ylim(-3.5, 3.5)
ax_pull.yaxis.set_minor_locator(mpltick.AutoMinorLocator(2))
# ax_pull.tick_params(axis="y", which="minor", left=False, right=False)
ax_pull.set_yticks(list(range(-2, 3, 2)))
ax_pull.grid(True, which="both", axis="y", ls="--", color="grey")
# Print stats
print("% pull < 1 sigma : ", len(pulls[pulls <= 1]) / len(pulls) * 100)

# Format axes
ax.set_xlabel("")
ax_pull.set_xlabel(r"$E_{x}$ [MeV]")
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

# Legend
handles, labels = ax.get_legend_handles_labels()
ax.legend(
    handles=[handles[-1]] + handles[:-1],
    labels=[labels[-1]] + labels[:-1],
    loc="upper right",
    labelspacing=0.4,
    # borderaxespad=0.3,
    # bbox_to_anchor=(1.02, 1),
)
fig.savefig(sty.thesis + "20O_dt_ex.pdf", dpi=300)
plt.show()
