import pyphysics as phys
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import matplotlib.ticker as mpltick
import sys
import uncertainties as un

sys.path.append("../")
sys.path.append("./")
import styling as sty
import dt

fig = plt.figure(figsize=(4.5, 6))
gs = gridspec.GridSpec(2, 1)
ax_l2 = fig.add_subplot(gs[0, 0])
ax_l0 = fig.add_subplot(gs[1, 0], sharex=ax_l2)

# l = 2
# ax_l2.tick_params(axis="y", which="both", right=False)
dt.unrebin.plot_exp("g0", ax=ax_l2)
ax_l2.set_prop_cycle(sty.cyclers["l012"])
dt.unrebin.remove_model("g0", "l = 2 ZR 2FNR")
dt.unrebin.remove_model("g0", "DA1p")
dt.unrebin.remove_model("g0", "Haixia")
dt.unrebin.remove_model("g0", "HT1p")
dt.unrebin.plot_models("g0", ax=ax_l2)
ax_l2.legend(loc="upper right")
ax_l2.set_ylim(0, 60)

# l = 0
# ax_l0.yaxis.tick_right()
dt.unrebin.plot_exp("g1", ax=ax_l0)
ax_l0.set_prop_cycle(sty.cyclers["l012"])
dt.unrebin.plot_models("g1", ax=ax_l0)
ax_l0.set_xlabel("")
ax_l0.set_ylabel("")
ax_l0.set_ylim(0, 2)

## Annotations
anns = [
    ("g.s", dt.unrebin.get_model("g0", "l = 2")),
    (dt.fit.get("g1")[0], dt.unrebin.get_model("g1", "l = 0")),
]

for i, ax in enumerate([ax_l2, ax_l0]):
    ex, sf = anns[i]
    ax.annotate(
        f"{ex}" if isinstance(ex, str) else f"{ex.n:.2f} MeV",
        xy=(0.5 if i != 0 else 0.4, 0.9),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=14,
    )
    ax.annotate(
        rf"C$^2$S = {sf.fSF:.2uS}",
        xy=(0.5 if i != 0 else 0.4, 0.825),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=14,
    )

# Common axis settings
for ax in [ax_l2, ax_l0]:
    # ax.set_yscale("log")
    ax.set_xlim(4, 17)

# Figure settings
fig.supxlabel(r"$\theta_{\text{CM}}$ [$\circ$]", fontsize=18)
fig.supylabel(r"d$\sigma$/d$\Omega$ [mb/sr]", fontsize=18)
fig.tight_layout()
fig.subplots_adjust(left=0.175, bottom=0.09, hspace=0)
fig.savefig("/media/Data/Docs/SSW/figures/ang0.png", dpi=300)

plt.show()

fig, axs = plt.subplots(5, 2, sharex=True, figsize=(5, 8))
states = ["g2", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7"]
handles = []
for i, state in enumerate(states):
    ax: mplaxes.Axes = axs.flat[i]
    models, q = dt.assignments[state]
    models.plot_exp(state, ax=ax)
    models.format_ax(state, ax)
    ax.set_prop_cycle(sty.cyclers["l012"])
    hand = models.plot_models(state, ax=ax)
    if i == 0:
        handles = hand
    # Axis settings
    ax.set_xlabel("")
    ax.set_ylabel("")
    ax.set_xlim(5, 16)
    if i % 2 != 0:
        ax.yaxis.tick_right()
    if i % 2 == 0:
        ax.tick_params(axis="y", which="both", right=False)
    ## Annotate
    ex, _ = dt.fit.get(state)
    ax.annotate(
        f"{un.nominal_value(ex):.2f} MeV",
        xy=(0.5, 0.875),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=10,
    )
    sf = models.get_model(state, "l = 1")
    if sf is not None:
        sf = sf.fSF
        ax.annotate(
            rf"C$^2$S = {sf:.2uS}",
            xy=(0.5, 0.775),
            xycoords="axes fraction",
            ha="center",
            va="center",
            fontsize=10,
        )
    ax.yaxis.set_major_locator(mpltick.MaxNLocator(nbins=3, integer=True))
axs.flat[-1].set_axis_off()
axs.flat[-1].legend(handles, [h.get_label() for h in handles], loc="center")

fig.tight_layout()
fig.supxlabel(r"$\theta_{\text{CM}}$ [$\circ$]", fontsize=18)
fig.supylabel(r"d$\sigma$/d$\Omega$ [mb/sr]", x=0.0, fontsize=18)
fig.subplots_adjust(left=0.12, bottom=0.075, wspace=0.0, hspace=0.0)
fig.savefig("/media/Data/Docs/SSW/figures/ang1.png", dpi=600)
# plt.close(fig)

plt.show()
