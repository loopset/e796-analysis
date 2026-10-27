import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from adjustText import adjust_text

LevelSet = list[tuple[float, str]]

ccei: LevelSet = [
    (0.00, r"$0^+$"),
    (1.70, r"$2^+$"),
    (2.28, r"$2^+$"),
    (3.17, r"$4^+$"),
    (3.83, r"$1^+$"),
    (3.99, r"$0^+$"),
    (4.02, r"$3^+$"),
    (4.10, r"$2^+$"),
    (4.52, r"$4^+$"),
]

exp: LevelSet = [
    (0.00, r"$0^+$"),
    (1.585, r"$2^+$"),
    (2.504, r"$2^+$"),
    (4, r"$?$"),
]

wbt: LevelSet = [
    (0.00, r"$0^+$"),
    (2.114, r"$2^+$"),
    (3.639, r"$2^+$"),
    (3.988, r"$0^+$"),
    (4.864, r"$4^+$"),
    (4.915, r"$3^+$"),
    (4.972, r"$2^+$"),
    (4.975, r"$\mathbf{1^-}$"),
]

threeb: LevelSet = [
    (0.00, r"$0^+$"),
    (1.113, r"$2^+$"),
    (2.175, r"$4^+$"),
    (3.681, r"$1^+$"),
    (3.854, r"$2^+$"),
    (4.069, r"$2^+$"),
    (4.588, r"$0^+$"),
    (4.752, r"$0^+$"),
    (4.818, r"$3^+$"),
    (4.831, r"$2^+$"),
    (5.146, r"$\mathbf{1^-}$"),
]

columns = [
    ("Exp", exp, "black"),
    # ("WBT", wbt, "dodgerblue"),
    # ("Three-body", threeb, "forestgreen"),
    ("SFO-tls", None, "none"),
    # ("CCEI", ccei, "crimson"),
]

sfo = phys.ShellModel(
    [
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j0n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j0p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j2n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j2p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j4n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j4p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j6n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j6p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j8n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j8p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j10n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j10p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j12n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j12p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j14n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j14p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j16n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j16p_j3p.txt",
    ],
    is_adding=True,
)
# Set min C2S
sfo.set_min_SF(0.15)
# Group by Ex and Jpif
groupexjpi = sfo.group_by_Ex_and_Jpif()


bar_half_width = 0.3
label_offset = 0.35
y_max = 12


plt.rcParams["xtick.labelsize"] = 16
fig, ax = plt.subplots(figsize=(6, 4), constrained_layout=True)
ax.set_xlim(-0.8, len(columns) - 1 + 0.9)
ax.set_ylim(-0.3, y_max)

texts = []
for x, (_, levels, color) in enumerate(columns):
    if levels is not None:
        for energy, label in levels:
            ax.hlines(
                energy,
                x - bar_half_width,
                x + bar_half_width,
                color=color,
                linewidth=1.5,
            )
            t = ax.annotate(
                label,
                xy=(x + label_offset, energy),
                va="center",
                ha="left",
                fontsize=10,
                color="black",
            )
            texts.append(t)
    else:
        _, t = sfo.plot_bars(
            groupexjpi, ax=ax, x=x, fontsize=10, right_padding=0.1, left_padding=0.1
        )
        texts += t

ax.spines["right"].set_visible(False)
ax.spines["top"].set_visible(False)
ax.spines["bottom"].set_visible(False)
ax.tick_params(axis="x", which="both", bottom=False, top=False, labelbottom=True)
ax.tick_params(axis="y", which="both", right=False)

# Sn and S2n
c18 = phys.Particle("18C")
ax.axhline(c18.get_sn(), ls="--", color="grey")
ax.axhline(c18.get_s2n(), ls="--", color="hotpink")
ax.annotate(
    r"$S_n$", xy=(-0.45, c18.get_sn() - 0.5), ha="center", va="center", fontsize=14
)
ax.annotate(
    r"$S_{2n}$", xy=(-0.45, c18.get_s2n() + 0.5), ha="center", va="center", fontsize=14
)

# Axis settings
ax.set_ylabel(r"$E_x$ [MeV]")
ax.set_title(r"$^{18}$C", fontweight="bold")
ax.set_xlim(-0.6, 1.5)
ax.set_xticks(range(len(columns)))
ax.set_xticklabels([name for name, _, _ in columns])
ax.legend(
    loc="upper left", fontsize=12, frameon=False, handlelength=1.5, handleheight=1.5
)

# Adjust texts
positions = [t.get_position() for t in texts]
adjust_text(
    texts,
    target_x=[p[0] for p in positions],
    target_y=[p[1] for p in positions],
    avoid_self=False,
    only_move=dict(text="y", static="y", explode="y", pull="y"),
    # arrowprops=dict(arrowstyle="-", color="crimson", ls="dotted"),
)
# ax.xaxis.set_ticks_position("top")
# ax.xaxis.set_label_position("top")


fig.savefig("./Outputs/levels_18C_slides.png", dpi=300)
# fig.savefig("./Outputs/levels_18C.pdf", dpi=300)
plt.show()
