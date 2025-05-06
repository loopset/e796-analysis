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

figs = []
dashed = False
for i, lst in enumerate(states):
    fig, axs = plt.subplots(2, 3, sharex=True, figsize=(9, 7))
    figs.append(fig)
    for j, state in enumerate(lst):
        ax: mplaxes.Axes = axs.flatten()[j]
        obj = which.get(state)
        if obj is None:
            print(f"State {state} doesn't have a dataset assigned")
            continue
        obj.plot_exp(state, ax)
        # Set colors
        ax.set_prop_cycle(sty.cyclers["l012"])
        # And ls
        if state == "v4":
            dashed = True
        models = obj.plot_models(state, ax, ls="dashed" if dashed else "solid")
        # Text annotation
        ex, _ = fit.get(state)
        text = (r"E$_{\mathrm{x}} = $ " + f"{ex.n:.2f}") if state != "g0" else "g.s"
        pos = (0.65, 0.925) if state != "g0" else (0.4, 0.925)
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
            mpltick.MaxNLocator(nbins=4, steps=[1, 2, 5, 10], integer=True)
        )
        if i == 0 and j == 0:
            ax.legend(models, [r"$\ell$ = 0", r"$\ell$ = 1", r"$\ell$ = 2"])


# Figure settings
for i, fig in enumerate(figs):
    fig.tight_layout()
    fig.subplots_adjust(hspace=0.025, wspace=0.175, bottom=0.085, left=0.07)
    fig.text(
        0.525,
        0.03,
        r"$\theta_{\mathrm{CM}}$ [$^{\circ}$]",
        ha="center",
        va="center",
        fontsize=18,
    )
    fig.text(
        0.02,
        0.55,
        r"$\mathrm{d}\sigma/\mathrm{d}\Omega$ [mb/sr]",
        rotation="vertical",
        ha="center",
        va="center",
        fontsize=18,
    )
    fig.savefig(f"./Outputs/ang_{i}.pdf")
    fig.savefig(f"./Outputs/ang_{i}.eps")
    # fig.tight_layout()

# for i, fig in enumerate(figs):
#     if i > 0:
#         plt.close(fig)

plt.show()