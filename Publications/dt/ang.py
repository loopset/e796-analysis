import pyphysics as phys
from pyphysics.actroot_interface import FitInterface, SFInterface
import uncertainties as un
import matplotlib.ticker as mpltick
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt
import sys

sys.path.append("../")
import styling as sty

# Read data
fit = FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
sfs = SFInterface("../../Fits/dt/Outputs/sfs.root")
sfs.remove_model("g0", "l = 2 ZR 2FNR")
rsfs = SFInterface("../../Fits/dt/Outputs/rebin_sfs.root")

# List of states per figure
# states = [["g0", "g1", "g2", "v0"], ["v1", "v2", "v3", "v4"], ["v5", "v6", "v7", "v8"]]
states = [["g0", "g1", "g2", "v0", "v1", "v2"], ["v3", "v4", "v5", "v6", "v7", "v12"]]
which = {
    "g0": sfs,
    "g1": sfs,
    "g2": sfs,
    "v0": sfs,
    "v1": rsfs,
    "v2": rsfs,
    "v3": rsfs,
    "v4": rsfs,
    "v5": rsfs,
    "v6": rsfs,
    "v7": rsfs,
    "v12": rsfs,
}
withl0 = {
    "g0": False,
    "g1": True,
    "g2": False,
    "v0": False,
    "v1": False,
    "v2": False,
    "v3": False,
    "v4": False,
    "v5": True,
    "v6": True,
    "v7": True,
    "v12": True,
}

# X axis limits
xlims = (4, 17)

fig, axs = plt.subplots(6, 2, sharex=True, figsize=(6, 8))
handles = []
for i, state in enumerate(sfs.fSFs):
    ax: mplaxes.Axes = axs.flatten()[i]
    obj = which.get(state)  # type: ignore
    if obj is None:
        print(f"State {state} doesn't have a dataset assigned")
        continue
    obj: SFInterface

    # Experimental
    obj.plot_exp(state, ax)
    obj.format_ax(state, ax)
    ax.set_xlabel("")
    ax.set_ylabel("")

    ## Axis formatting
    # 1-> Labels on the right
    if i % 2 == 1:  # right column
        ax.yaxis.tick_right()
        ax.tick_params(axis="y", which="both", left=False, right=True)
        ax.yaxis.set_label_position("right")
    else:
        ax.tick_params(axis="y", which="both", left=True, right=False)
    # 2-> Bottom/top ticks and axis break
    d = 0.5  # proportion of vertical to horizontal extent of the slanted line
    abreak = dict(
        marker=[(-1, -d), (1, d)],
        markersize=12,
        linestyle="none",
        color="k",
        mec="k",
        mew=1,
        clip_on=False,
    )
    if 0 <= i <= 1:
        ax.tick_params(axis="x", which="both", bottom=False, top=True)
        ax.spines.bottom.set_visible(False)
        ax.plot([0, 1], [0, 0], transform=ax.transAxes, **abreak)  # type: ignore
    elif 10 <= i <= 11:
        ax.tick_params(axis="x", which="both", bottom=True, top=False, labelbottom=True)
        ax.spines.top.set_visible(False)
        # ax.plot([0], [0], transform=ax.transAxes, **abreak)  # type: ignore
        ax.plot([0, 1], [1, 1], transform=ax.transAxes, **abreak)  # type: ignore
    else:
        ax.tick_params(axis="x", which="both", bottom=False, top=False)
        ax.spines.top.set_visible(False)
        ax.spines.bottom.set_visible(False)
        ax.plot([0, 1], [0, 0], transform=ax.transAxes, **abreak)  # type: ignore
        ax.plot([0, 1], [1, 1], transform=ax.transAxes, **abreak)  # type: ignore

    ## Models
    obj.fSFs[state] = [model for model in obj.fSFs[state] if "=" in model.fName]
    l0 = withl0.get(state)
    if l0 is None:
        raise ValueError(f"Must specify withl0 for {state} state")
    if l0:
        ax.set_prop_cycle(sty.cyclers["l012"])
    else:
        ax.set_prop_cycle(sty.cyclers["l12"])
        obj.remove_model(state, "l = 0")
    models = obj.plot_models(state, ax)

    # Text annotation
    ex, _ = fit.get(state)
    text = (
        (r"E$_{\mathrm{x}} = $ " + f"{un.nominal_value(ex):.2f}")
        if state != "g0"
        else "g.s"
    )
    pos = (0.65, 0.85) if state != "g0" else (0.4, 0.85)
    ax.annotate(
        text,
        xy=pos,
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=14,
    )
    # Axis limits
    ax.set_xlim(*xlims)
    # Locator
    ax.yaxis.set_major_locator(
        mpltick.MaxNLocator(nbins=3, steps=[1, 2, 5, 10], integer=True, min_n_ticks=1)
    )
    if i == 1:
        handles = models

# Hide last one
# axs.flat[-1].axis("off")
axs.flat[0].legend(
    handles,
    [r"$L = 0$", r"$L = 1$", r"$L = 2$"],
    labelspacing=0.075,
    borderpad=0.2,
    loc="upper right",
)

# Figure settings
fig.tight_layout()
fig.subplots_adjust(hspace=0.05, wspace=0.0, bottom=0.075, left=0.085)
fig.text(
    0.525,
    0.03,
    r"$\theta_{CM}$ [$\circ$]",
    ha="center",
    va="center",
    fontsize=18,
)
fig.text(
    0.03,
    0.55,
    r"$d\sigma/d\Omega$ [$mb\cdot sr^{-1}$]",
    rotation="vertical",
    ha="center",
    va="center",
    fontsize=18,
)
plt.show()
