import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

# --- Config ---
STABLE_COLOR = "darkgrey"  # <-- tune this
# --------------

Z = 8
isotopes = [
    {"A": 16, "stable": True},
    {"A": 17, "stable": True},
    {"A": 18, "stable": True},
    {"A": 19, "stable": False},
    {"A": 20, "stable": False},
    {"A": 21, "stable": False},
    {"A": 22, "stable": False},
    {"A": 23, "stable": False},
    {"A": 24, "stable": False},
]

BOX = 0.6
PAD = 0

fig, ax = plt.subplots(figsize=(9, 2.5), constrained_layout=True)

for i, iso in enumerate(isotopes):
    x = i * (BOX + PAD)
    fc = STABLE_COLOR if iso["stable"] else "white"
    tc = "black" if iso["stable"] else "black"

    ax.add_patch(
        mpatches.FancyBboxPatch(
            (x, 0),
            BOX,
            BOX,
            boxstyle="square,pad=0",
            linewidth=1.5,
            edgecolor="black",
            facecolor=fc,
        )
    )

    ax.annotate(
        f"$^{{{iso['A']}}}$O",
        xy=(x + BOX / 2, BOX / 2),
        ha="center",
        va="center",
        fontsize=18,
        fontweight="bold",
        color=tc,
    )

    # Add neutron number label below
    N = iso["A"] - Z
    ax.annotate(
        f"{N}",
        xy=(x + BOX / 2, -0.075),
        ha="center",
        va="top",
        fontsize=16,
        color="black",
    )

n = len(isotopes)
ax.set_xlim(-0.5, n * (BOX + PAD) - PAD + 0.05)

# Add N = label to the left
ax.annotate(
    "N =",
    xy=(-0.2, -0.075),
    ha="center",
    va="top",
    fontsize=16,
    color="black",
)

# Add Z = 8 label vertically centered to the left
ax.annotate(
    "Z = 8",
    xy=(-0.2, BOX / 2),
    ha="center",
    va="center",
    fontsize=16,
    rotation=90,
    color="black",
)
ax.set_ylim(-0.35, BOX + 0.05)
ax.set_aspect("equal")
ax.axis("off")

# plt.tight_layout()
fig.savefig("./Outputs/oxygen_chart.png", dpi=300, bbox_inches="tight")

plt.show()
