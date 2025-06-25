from collections import defaultdict
from typing import Dict, List, Tuple

import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.patches as mplpat
from adjustText import adjust_text
from dataclasses import dataclass

# Define some constants
LGAP = 0.025
JGAP = 0.135


@dataclass
class HO:
    N: int

    def pos(self) -> float:
        return float(self.N)

    def deg(self) -> int:
        return self.N // 2

    def format(self) -> str:
        return f"{self.N}"


@dataclass
class WSl2:
    n: int  # actual n, count of state with given l
    l: int
    parent: HO

    def pos(self) -> float:
        if self.parent.deg() > 0:
            return self.parent.N - self.l * (self.l + 1) * LGAP
        else:
            return self.parent.pos()

    def format(self) -> str:
        return f"{self.n}{phys.QuantumNumbers.letters.get(self.l)}"


@dataclass
class SO:
    j: float
    parent: WSl2

    def pos(self) -> float:
        if self.parent.l > 0:
            gap = (
                (self.parent.l + 1) / 2
                if (self.parent.l - self.j) > 0
                else -self.parent.l / 2
            ) * JGAP
            return self.parent.pos() + gap
        else:
            return self.parent.pos()

    def get_q(self) -> phys.QuantumNumbers:
        return phys.QuantumNumbers(self.parent.n, self.parent.l, self.j)

    def format(self) -> str:
        return self.get_q().format()


## Harmonic oscillator
nho = 5
ho = [HO(i) for i in range(nho)]

## WS plus l2 term
wsl: List[WSl2] = []
neven = 0
nodd = 0
for parent in ho:
    vals = []
    if parent.N % 2 == 0:  # pair
        neven += 1
        vals = [2 * i for i in range(neven)]
    else:
        nodd += 1
        vals = [2 * i + 1 for i in range(nodd)]
    wsl.extend([WSl2(-1, l, parent) for l in vals])

l_dict: Dict[int, int] = defaultdict(int)
for val in wsl:
    l_dict[val.l] += 1
    val.n = l_dict[val.l]

## Spin-orbit
so: List[SO] = []
for val in wsl:
    if val.l > 0:
        for sign in [-1, +1]:
            j = val.l + sign * 0.5
            so.append(SO(j, val))
    else:
        so.append(SO(0.5, val))


fig, ax = plt.subplots()
# Parameters of plotting
width = 0.6

# Save annotations
anns = []
reperes = []
# Harmonic oscillator
x = 0
for parent in ho:
    ax.plot([x - width / 2, x + width / 2], [parent.pos()] * 2, color="black")
    ax.annotate(
        parent.format(),
        xy=(x - width / 2 - 0.1, parent.N),
        ha="center",
        va="center",
        fontsize=12,
    )

# WS + l2
x = 1
aux_wsl: Dict[int, int] = defaultdict(int)
for val in wsl:
    key = val.parent.N
    aux_wsl[key] += 1
    ax.plot([x - width / 2, x + width / 2], [val.pos()] * 2, color="dodgerblue")
    offset = +0.075 if aux_wsl[key] == 1 else -0.11
    ax.annotate(
        val.format(),
        xy=(x, val.pos() + offset),
        ha="center",
        va="center",
        fontsize=12,
    )
    # Lines connecting them
    xleft = (x - 1) + width / 2
    yleft = val.parent.pos()
    xright = x - width / 2
    yright = val.pos()
    ax.plot([xleft, xright], [yleft, yright], color="dodgerblue", ls="--", alpha=0.5)

# SO
x = 2
for val in so:
    ax.plot([x - width / 2, x + width / 2], [val.pos()] * 2, color="crimson")
    ann = ax.annotate(
        val.format(),
        xy=(x + width / 2 + 0.15, val.pos()),
        ha="center",
        va="center",
        fontsize=10,
    )
    anns.append(ann)
    reperes.append((x + width / 2 + 0.01, val.pos()))
    # Lines connecting them
    xleft = (x - 1) + width / 2
    yleft = val.parent.pos()
    xright = x - width / 2
    yright = val.pos()
    ax.plot([xleft, xright], [yleft, yright], color="crimson", ls="--", alpha=0.5)

## Magic numbers
radius = 0.1
# HO magic
ho_magic = [2, 8, 20, 40]
for i, (low, up) in enumerate(list(zip(ho[:-1], ho[1:]))):
    y = (up.pos() + low.pos()) / 2
    # circle = mplpat.Circle((0, y), radius=radius, color="skyblue", ec="black")
    # ax.add_patch(circle)
    ax.annotate(
        f"{ho_magic[i]}",
        xy=(0, y),
        ha="center",
        va="center",
        fontsize=14,
        style="italic"
    )
so_magic: Dict[int, Tuple[phys.QuantumNumbers, phys.QuantumNumbers]] = {
    2: (phys.QuantumNumbers(1, 0, 0.5), phys.QuantumNumbers(1, 1, 1.5)),
    8: (phys.QuantumNumbers(1, 1, 0.5), phys.QuantumNumbers(1, 2, 2.5)),
    20: (phys.QuantumNumbers(1, 2, 1.5), phys.QuantumNumbers(1, 3, 3.5)),
    28: (phys.QuantumNumbers(1, 3, 3.5), phys.QuantumNumbers(2, 1, 1.5)),
    50: (phys.QuantumNumbers(1, 4, 4.5), phys.QuantumNumbers(2, 2, 2.5)),
}
for magic, tup in so_magic.items():
    low = 0
    up = 0
    for val in so:
        if val.get_q() == tup[0]:
            low = val.pos()
        elif val.get_q() == tup[1]:
            up = val.pos()
        else:
            continue
    ax.annotate(
        f"{magic}",
        xy=(2, (low + up) / 2),
        fontsize=14,
        ha="center",
        va="center",
        style="italic"
    )

# Draw column labels
for i, label in enumerate(["H.O.", r"$\ell^2$", r"$\vec{\ell}\cdot\vec{s}$"]):
    ax.annotate(label, xy=(i, -0.5), ha="center", va="center", fontsize=14)
    if i < 2:
        ax.annotate("+", xy=(i + 0.5, -0.5), ha="center", va="center", fontsize=14)


# Disable axis
ax.set_axis_off()

# Axis ranges
ax.set_xlim(-0.5, 2.5)
ax.set_ylim(-0.75, 4.5)

# Adjust text
adjust_text(
    anns,
    target_x=[val[0] for val in reperes],
    target_y=[val[1] for val in reperes],
    avoid_self=False,
    only_move=dict(text="y", static="y", explode="y", pull="y"),
    arrowprops=dict(arrowstyle="-", color="crimson", ls="dotted"),
)

fig.tight_layout()
fig.savefig("./Outputs/shell_model.pdf")
fig.savefig("./Outputs/shell_model.png")
plt.show()
