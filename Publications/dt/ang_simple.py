from turtle import right
import pyphysics as phys
import uproot
import matplotlib.ticker as mpltick
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt
import sys

sys.path.append("../")
import styling as sty

# Read data
fit = phys.FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
sfs = phys.SFInterface("../../Fits/dt/Outputs/sfs.root")
sfs.remove_model("g0", "l = 2 ZR 2FNR")
rsfs = phys.SFInterface("../../Fits/dt/Outputs/rebin_sfs.root")

# List of states per figure
# states = [["g0", "g1", "g2", "v0"], ["v1", "v2", "v3", "v4"], ["v5", "v6", "v7", "v8"]]
states = [["g0", "g1", "g2", "v0", "v1", "v2"], ["v3", "v4", "v5", "v6", "v7", "v8"]]
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
    "v8": rsfs,
}

# X axis limits
xlims = (4, 17)

fig, axs = plt.subplots(6, 2, sharex=True, figsize=(6, 8))
dashed = False
for i, state in enumerate(sfs.fSFs):
    ax: mplaxes.Axes = axs.flatten()[i]
    obj = which.get(state)
    if obj is None:
        print(f"State {state} doesn't have a dataset assigned")
        continue

    ## Experimental
    obj.plot_exp(state, ax)

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
        ax.plot([0, 1], [0, 0], transform=ax.transAxes, **abreak) #type: ignore
    elif 10 <= i <= 11:
        ax.tick_params(axis="x", which="both", bottom=True, top=False)
        ax.spines.top.set_visible(False)
        ax.plot([0, 1], [1, 1], transform=ax.transAxes, **abreak) #type: ignore
    else:
        ax.tick_params(axis="x", which="both", bottom=False, top=False)
        ax.spines.top.set_visible(False)
        ax.spines.bottom.set_visible(False)
        ax.plot([0, 1], [0, 0], transform=ax.transAxes, **abreak) #type: ignore
        ax.plot([0, 1], [1, 1], transform=ax.transAxes, **abreak) #type: ignore

    ## Models
    # Set colors
    ax.set_prop_cycle(sty.cyclers["l012"])
    # And ls
    if state == "v4":
        dashed = True
    models = obj.plot_models(state, ax, ls="dashed" if dashed else "solid")

    ## Others
    # Text annotation
    ex, _ = fit.get(state)
    text = (r"E$_{\mathrm{x}} = $ " + f"{ex.n:.2f}") if state != "g0" else "g.s"
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
    # Axis break

    if i == 0:
        ax.legend(models, [r"$\ell$ = 0", r"$\ell$ = 1", r"$\ell$ = 2"], labelspacing=0.05, borderpad=0.2)


# Figure settings
fig.tight_layout()
fig.subplots_adjust(hspace=0.05, wspace=0.0, bottom=0.075, left=0.085)
# fig.tight_layout()
fig.text(
    0.525,
    0.03,
    r"$\theta_{\mathrm{CM}}$ [$^{\circ}$]",
    ha="center",
    va="center",
    fontsize=18,
)
fig.text(
    0.03,
    0.55,
    r"$\mathrm{d}\sigma/\mathrm{d}\Omega$ [mb/sr]",
    rotation="vertical",
    ha="center",
    va="center",
    fontsize=18,
)
fig.savefig("./Outputs/ang_simple.pdf")
fig.savefig("./Outputs/ang_simple.eps")
plt.show()
