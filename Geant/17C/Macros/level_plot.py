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
    ("CCEI", ccei, "crimson"),
    ("Three-body", threeb, "forestgreen"),
    ("WBT", wbt, "dodgerblue"),
    ("Exp", exp, "black"),
]

bar_half_width = 0.35
label_offset = 0.4
y_max = 7


plt.rcParams["xtick.labelsize"] = 16
fig, ax = plt.subplots(figsize=(7, 4), constrained_layout=True)
ax.set_xlim(-0.8, len(columns) - 1 + 0.9)
ax.set_ylim(-0.3, y_max)

texts = []
for x, (_, levels, color) in enumerate(columns):
    for energy, label in levels:
        ax.hlines(
            energy, x - bar_half_width, x + bar_half_width, color=color, linewidth=1.5
        )
        t = ax.annotate(
            label,
            xy=(x + label_offset, energy),
            va="center",
            ha="left",
            fontsize=12,
            color="black",
        )
        texts.append(t)

# Sn
# In the order defined by the xticks
## CCEI, Three-body, WBT, Exp
sns = [2.4, 4.4, 5.0, 4.18]
for i, sn in enumerate(sns):
    ax.fill_between(
        [i - bar_half_width, i + bar_half_width],
        sn,
        y_max,
        color="lightgray",
        ec="none",
        alpha=0.5,
        label=r"$S_n$" if i == 0 else None,
    )
# S2n
s2ns = [np.nan, 5.3, 5.1, 4.92]
for i, s2n in enumerate(s2ns):
    if not np.isnan(s2n):
        ax.fill_between(
            [i - bar_half_width, i + bar_half_width],
            s2n,
            y_max,
            color="lightpink",
            ec="none",
            alpha=0.5,
            label=r"$S_{2n}$" if i == 1 else None,
        )

ax.set_ylabel(r"$E_x$ [MeV]")
ax.set_yticks(range(0, int(y_max) + 1))

ax.set_title(r"$^{18}$C", fontweight="bold")
ax.set_xticks(range(len(columns)))
ax.set_xticklabels([name for name, _, _ in columns])
ax.tick_params(axis="x", which="minor", bottom=False, top=False)

ax.legend(
    loc="upper left", fontsize=14, frameon=False, handlelength=1.5, handleheight=1.5
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


fig.savefig("./Outputs/levels_18C.png", dpi=300)
fig.savefig("./Outputs/levels_18C.pdf", dpi=300)
plt.show()
