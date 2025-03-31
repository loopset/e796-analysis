from shutil import which
from turtle import right
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
from interfaces import FitInterface

plt.style.use("../../Python/actroot.mplstyle")


# Read the energies
fit = FitInterface(
    "./Outputs/interface.root", "./Outputs/fit_juan_RPx.root", "./Outputs/sfs.root"
)
es = [d.Ex for _, d in fit.data.items()]


# Read the model
class State:
    def __init__(self, j: float, pi: int, n: int, ex: float):
        self.fJ = j
        self.fPi = pi
        self.fN = n
        self.fEx = ex

    def __str__(self):
        return f"-- State --\n j : {self.fJ} pi : {self.fPi} n : {self.fN}\n Ex : {self.fEx} MeV"

    def format(self) -> str:
        sign = "+" if self.fPi > 0 else "-"
        return rf"{{{int(self.fJ)}}}^{{{sign}}}_{{{self.fN}}}"


def parse_summary(file: str) -> list:
    with open(file) as f:
        lines = f.readlines()
    ret = []
    for line in lines:
        if len(line) < 70:
            continue  ## not state line
        j = float(line[10])
        pi = int(1 if line[12] == "+" else -1)
        n = int(line[17:19])
        ex = float(line[39:45])
        state = State(j, pi, n, ex)
        ret.append(state)
        # print("========")
        # print("J : ", line[10])
        # print("Pi : ", line[12])
        # print("N : ", line[17:19])
        # print("Ex : ", line[39:45])
    return ret


# ENSDF dataset
ensdf = [
    State(0, +1, 1, 0),
    State(2, +1, 1, 1.673),
    State(4, +1, 1, 3.570),
    State(2, +1, 2, 4.072),
    State(0, +1, 2, 4.456),
    State(4, +1, 2, 4.850),
    5.002,
    State(2, +1, 3, 5.234),
    State(2, +1, 4, 5.304),
    State(0, +1, 3, 5.387),
    State(3, -1, 1, 5.614),
    6.555,
    State(5, -1, 1, 7.252),
    7.622,
    State(4, +1, 3, 7.754),
    State(5, -1, 2, 7.855),
    State(4, +1, 4, 8.554),
    State(3, -1, 2, 8.804),
    State(0, +1, 4, 8.962),
    State(0, +1, 5, 9.770),
    State(2, +1, 5, 10.125),
]

## SFO-tls
sfotls = parse_summary("./Inputs/SM/summary_O20_sfotls_mod.txt")
theo = np.array([s.fEx for s in sfotls])
closest = [sfotls[np.abs(theo - e).argmin()] for e in es]

# Draw
fig, ax = plt.subplots(1, 1, figsize=(9, 5))
ax: plt.Axes

# State width
width = 0.75

# Draw experimental
for e in es:
    x = [1 - width / 2, 1 + width / 2]
    y = [e.n, e.n]
    ax.plot(x, y, lw=2, marker="", color="dodgerblue")

# Draw theoretical
for state in sfotls:
    if state.fEx > 12:
        continue
    if state.fN > 6:
        continue
    x = [2 - width / 2, 2 + width / 2]
    y = [state.fEx, state.fEx]
    ax.plot(x, y, lw=1.25, marker="", color="violet")

# Draw states closer to ones
for state in closest:
    x = [2 - width / 2, 2 + width / 2]
    y = [state.fEx, state.fEx]
    ax.plot(x, y, lw=2, marker="", color="crimson")
    ax.annotate(
        f"${state.format()}$",
        xy=(x[1] + 0.05, y[1]),
        ha="center",
        va="center",
        fontsize=14,
    )

# Draw ENSDF
toggle = False
for state in ensdf:
    x = [3 - width / 2, 3 + width / 2]
    if isinstance(state, State):
        y = [state.fEx, state.fEx]
    else:
        y = [state, state]
    ax.plot(x, y, lw=2, marker="", color="darkviolet")
    if isinstance(state, State):
        pos = (x[1] + 0.05, y[1])
        if toggle:
            pos = (x[0] - 0.05, y[0])
        ax.annotate(
            f"${state.format()}$",
            xy=pos,
            ha="center",
            va="center",
            fontsize=10,
        )
        toggle = not toggle

ax.set_xlim(0.5, 3.5)
ax.set_ylim(-0.1, 12)
ax.set_xticks([1, 2, 3], ["Exp", "SFO-tls", "ENSDF"], fontsize=18)
ax.tick_params(axis="x", which="both", top=None, bottom=None, pad=10)
ax.tick_params(axis="y", which="both", right=None)
ax.spines["bottom"].set_visible(False)
ax.spines["top"].set_visible(False)
ax.spines["right"].set_visible(False)
ax.set_ylabel(r"E$_{x}$ [MeV]")
ax.grid(visible=True, which="both", axis="y")
# Show
fig.tight_layout()
fig.savefig("./Pictures/March25/sfotls.png", dpi=200)
plt.show()
