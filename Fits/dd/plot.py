import uncertainties as un
import numpy as np
import uncertainties.umath as umath
import uncertainties.unumpy as unp
import math
import matplotlib.pyplot as plt
import matplotlib as mpl
import ROOT as r

import sys

sys.path.append("../../Python/")
plt.style.use("../../Python/actroot.mplstyle")

from interfaces import FitInterface

# Read the energies
fit = FitInterface(
    "./Outputs/interface.root", "./Outputs/fit_juan_RPx.root", "./Outputs/sfs.root"
)
es = [d.Ex for _, d in fit.data.items()]
# Labels
labels = {
    "g0": r"$0^+_1$",
    "g1": r"$2^+_1$",
    "g2": r"$2^+_2$",
    "g3": r"$3^-_1$",
    # "g5": r"$2^+_2 \otimes \lambda_2$",
    # "g8": r"$3^-_1 \otimes \lambda_2$",
    # "g9": r"$2^+_2 | 3^-_1 \otimes \lambda_{3|2}$",
}

# Compute the quotients
quotients = []
for i, ref in enumerate(es):
    if i == 0:
        continue  # ignore gs
    aux = []
    for j in range(1, i):
        aux.append(ref / es[j])
    quotients.append(aux)

# Get energy of first 2+
e2plus1 = es[1]
e3neg1 = es[3]
# Vibrational model l = 2
evib = []
for i in range(1, 4):
    evib.append(e2plus1 * i)

# Vibrational model l = 3
evib3 = []
for i in range(1, 3):
    evib3.append(e3neg1 * i)

# Rotational model
erot = []
for j in [2, 4, 6]:
    erot.append(e2plus1 * (j * (j + 1)) / (2 * (2 + 1)))

# Draw
fig, axs = plt.subplots(2, 5, figsize=(17, 6))

for i, q in enumerate(quotients):
    ax = axs.flat[i]
    ax: plt.Axes
    # Set axis labels
    x = [f"g{state + 1}" for state in range(len(q))]
    ax.errorbar(x, unp.nominal_values(q), yerr=unp.std_devs(q))
    # Set subplots title
    ax.set_title(f"g{len(q) + 1}")
    # Draw lines
    for l in range(2, 5):
        ax.axhline(y=l, marker="none", color=plt.cm.tab20(l / 5))

fig.tight_layout()
fig.savefig("./Pictures/March25/quotient_search.png")
plt.close(fig)

# Draw energies
fig, ax = plt.subplots(1, 1, figsize=(5, 5))
ax: plt.Axes
# Draw axhlines
# Vibrational
for ev in evib:
    ax.axhline(
        unp.nominal_values(ev),
        ls="dashed",
        lw=1.5,
        color="green",
        marker="none",
        zorder=1,
    )

for ev in evib3:
    ax.axhline(
        unp.nominal_values(ev),
        ls="dashed",
        lw=1.5,
        color="orange",
        marker="none",
        zorder=1,
    )
# Rotational
for er in erot:
    ax.axhline(
        unp.nominal_values(er),
        ls="dotted",
        lw=1.5,
        color="blue",
        marker="none",
        zorder=1,
    )
# Our states
for i, e in enumerate(es):
    ax.plot([0.25, 0.75], [e.n, e.n], marker="none", lw=2)
    key = f"g{i}"
    # Annotate
    if key in labels:
        ax.annotate(labels[key], xy=(0.8, e.n), ha="left", va="center", fontsize=16)
    ax.annotate(key, xy=(0.2, e.n), ha="right", va="center", fontsize=16)

# Limits
ax.set_xlim(0, 1.15)
# Axis
ax.set_ylabel(r"E$_{x}$ [MeV]")
ax.get_xaxis().set_visible(False)
# ax.grid(which="both", axis="y")
# Legend
entries = [
    mpl.lines.Line2D(
        [0], [0], lw=1.5, color="green", marker="none", ls="dashed", label="Vibrational L = 2"
    ),
    mpl.lines.Line2D(
        [0], [0], lw=1.5, color="orange", marker="none", ls="dashed", label="Vibrational L = 3"
    ),
    mpl.lines.Line2D(
        [0], [0], lw=1.5, color="blue", marker="none", ls="dotted", label="Rotational"
    ),
]
ax.legend(handles=entries, loc="lower left", bbox_to_anchor=(0.05, 1.01), fontsize=14, ncols=2)

fig.tight_layout()
fig.savefig("./Pictures/March25/tentative_assignments.png")

plt.show()
