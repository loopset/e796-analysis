from cProfile import label
import enum
import uncertainties as un
import numpy as np
import uncertainties.umath as umath
import uncertainties.unumpy as unp
import math
import matplotlib.pyplot as plt

import sys

sys.path.append("../../../Python/")

from particle import Particle
from bernstein import Bernstein

plt.style.use("../../../Python/actroot.mplstyle")

# Constants
p = Particle(8, 20)


# Functions
def BEL_to_beta(
    be: float | un.UFloat, z: int, a: int, l: float, up: bool = True
) -> float | un.UFloat:
    """Function that converts a B(EL) to betaL"""
    r = 1.2 * a ** (1 / 3)
    fspin = (2 * l + 1) / (
        2 * 0 + 1
    )  # assuming always final state in down transitions is gs
    if not up:
        be *= fspin
    beta = 4 * math.pi / (3 * z * r**l) * umath.sqrt(be)
    print("======================")
    print("B(EL)  : ", be)
    print(f"(Z,A) : ({z}, {a})")
    print("beta   : ", beta)
    return beta


def bernstein(
    em: float | un.UFloat, nucl: float | un.UFloat, nz: float, bpbn: float = 1
) -> float | un.UFloat:
    ratio = bpbn * (nucl / em * (1 + 1.0 / bpbn * nz) - 1)
    return ratio


## (p,p) dataset
pplabels = [r"E. Khan $2_{1}^{+}$", r"Reanalyzed $2_{1}^{+}$", r"Our $2_{1}^{+}$"]
ppem = [un.ufloat(0.261, 0.009), un.ufloat(0.268, 0.005), un.ufloat(0.268, 0.005)]
ppnuclear = [un.ufloat(0.55, 0.06), un.ufloat(0.5815, 0.0195), un.ufloat(0.4087, 0.0073)]
# Simplified formula
ppmm = [bernstein(em, nucl, p.fN / p.fZ, 1 / 3) for em, nucl in zip(ppem, ppnuclear)]
# Generalized formula
ppgen = []
for e, nu in zip(ppem, ppnuclear):
    b = Bernstein(p, nu, e, 1 / 3)
    ppgen.append(b.fMnMp)

## (d,d) dataset
ddlabels = [r"$2_{1}^{+}$", r"$2_{2}^{+}$", r"$3_{1}^{-}$"]
ls = [2, 2, 3]
isUp = [False, False, True]
ddem = [un.ufloat(5.9, 0.2), un.ufloat(1.3, 0.2), un.ufloat(1.19e3, 0.10e3)]
ddem = [BEL_to_beta(be, p.fZ, p.fA, ls[i], isUp[i]) for i, be in enumerate(ddem)]

ddnuclear = [
    un.ufloat(0.3170, 0.0063),
    un.ufloat(0.1828, 0.0051),
    un.ufloat(0.2445, 0.0069),
]
# Simplified formula
ddmm = [bernstein(e, nu, p.fN / p.fZ) for e, nu in zip(ddem, ddnuclear)]
# Using extended formula
ddgen = []
for e, nu in zip(ddem, ddnuclear):
    b = Bernstein(p, nu, e)
    ddgen.append(b.fMnMp)

# Compare with E. Khan's resutls
khannuclear = [un.ufloat(0.55, 0.06), np.nan, un.ufloat(0.35, 0.05)]
khanmm = [un.ufloat(2.35, 0.37) * 1.5, np.nan, np.nan]


# Figure
fig, axs = plt.subplots(2, 2, figsize=(11, 7))


def plot(ax: plt.Axes, x: list, y: list, legend: str, **kwargs) -> None:
    ax.errorbar(
        x,
        unp.nominal_values(y),
        yerr=unp.std_devs(y),
        ls="none",
        label=legend,
        **kwargs,
    )
    return


## (p,p) Betas
plot(axs[0][0], pplabels, ppem, r"EM")
plot(axs[0][0], pplabels, ppnuclear, r"Exp. (p,p)")
axs[0][0].annotate("Thesis value; others are I. Zanon",xy=(0.05, 0.275), xytext=(0.45, 0.4), ha="center", va="center",
                arrowprops=dict(arrowstyle="-"), fontsize=14)

## (p,p) MnMp
plot(axs[0][1], pplabels, ppmm, r"Simple Bernstein")
plot(axs[0][1], pplabels, ppgen, r"Generalized Bernstein")
# Horizontal span for E. Khan
ax = axs[0][1]
ax: plt.Axes
ax.axhspan(khanmm[0].n - khanmm[0].s, khanmm[0].n + khanmm[0].s, color="red", alpha=0.25, label="E. Khan")
ax.axhline(khanmm[0].n, ls="dashed", lw=1.25, marker="", color="crimson")
# plot(axs[0][1], pplabels[0], khanmm[0], r"E. Khan", color="red")

##(d,d) Betas
plot(axs[1][0], ddlabels[:2], ddem[:2], r"EM I. Zanon et al")
plot(axs[1][0], ddlabels[-1], ddem[-1], r"EM N. Nakatsuka et al")
plot(axs[1][0], ddlabels, ddnuclear, r"Our exp. (d,d)")

## (d,d) MnMp
plot(axs[1][1], ddlabels, ddmm, r"Simple Bernstein")
plot(axs[1][1], ddlabels, ddgen, r"Generalized Bernstein")
plot(axs[1][1], ddlabels[0], khanmm[0], r"E. Khan", color="red")
# Axis settings
# X limits
for i, labels in enumerate([pplabels, ddlabels]):
    for j in range(2):
        axs[i][j].set_xlim(-0.5, len(labels) - 1 + 0.5)

# Titles
for i in range(len(axs)):
    for j in range(2):
        if j == 0:
            axs[i][j].set_ylabel(r"$\beta_{L}$", fontsize=16)
        else:
            axs[i][j].set_ylabel(r"$M_{n} / M_{p}$", fontsize=16)

# Legends
for ax in axs.flatten():
    ax.legend(fontsize=12, frameon=True, fancybox=True, shadow=True, loc="best")

# Titles
axs[0][0].set_title(r"(p,p) $\beta$")
axs[0][1].set_title(r"(p,p) ratios")
axs[1][0].set_title(r"(d,d) $\beta$")
axs[1][1].set_title(r"(d,d) ratios")

# Figure saving
plt.tight_layout()
plt.savefig("./Pictures/beta_study.png", dpi=200)
plt.show()
