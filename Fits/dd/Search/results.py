from cProfile import label
import enum
import uncertainties as un
import numpy as np
import uncertainties.umath as umath
import uncertainties.unumpy as unp
import math
import matplotlib as mpl
import matplotlib.pyplot as plt

import sys

sys.path.append("../../../Python/")

from particle import Particle
from bernstein import Bernstein

plt.style.use("../../../Python/actroot.mplstyle")
plt.rcParams["lines.marker"] = "o"

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
pplabels = [r"E. Khan $2_{1}^{+}$", r"Reana $2_{1}^{+}$", r"Our $2_{1}^{+}$", r"E. Khan $3^{-}_{1}$",  r"Reana $3_{1}^{-}$",  r"Our $3_{1}^{-}$"]
paperbe3 = BEL_to_beta(un.ufloat(1.6e3, 0), p.fZ, p.fA, 3)
ppem = [un.ufloat(0.261, 0.009), un.ufloat(0.268, 0.005), un.ufloat(0.268, 0.005), paperbe3, paperbe3, paperbe3]
ppnuclear = [un.ufloat(0.55, 0.06), un.ufloat(0.5815, 0.0195), un.ufloat(0.4087, 0.0073), un.ufloat(0.35, 0.05), un.ufloat(0.331, 0.019), un.ufloat(0.238, 0.013)]
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
ddem = [un.ufloat(5.9, 0.2), un.ufloat(1.3, 0.2), un.ufloat(1.6e3, 0.)]
ddem = [BEL_to_beta(be, p.fZ, p.fA, ls[i], isUp[i]) for i, be in enumerate(ddem)]

# Using Daehnick normalization
# ddnuclear = [
#     un.ufloat(0.3170, 0.0063),
#     un.ufloat(0.1828, 0.0051),
#     un.ufloat(0.2445, 0.0069),
# ]
# Using PROTON normalization
ddnuclear = [
    un.ufloat(0.2618, 0.0052),
    un.ufloat(0.1523, 0.0042),
    un.ufloat(0.2054, 0.0058),
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


def plot(ax: plt.Axes, x: list, y: list, legend: str | None, **kwargs) -> None:
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
axs[0][0].annotate("B(E2) thesis; others I. Zanon",xy=(0.05, 0.275), xytext=(0.85, 0.35), ha="center", va="center",
                arrowprops=dict(arrowstyle="-"), fontsize=12)
axs[0][0].annotate("Theo value B(E3) = $1.6 \cdot 10^{3} \mathrm{e}^{2}\mathrm{fm}^{6}$ ",xy=(3.9, 0.57), ha="center", va="center", fontsize=11)
## (p,p) MnMp
plot(axs[0][1], pplabels, ppmm, r"Simple Bernstein")
plot(axs[0][1], pplabels, ppgen, r"Generalized Bernstein")
# Horizontal span for E. Khan
ax = axs[0][1]
ax: plt.Axes
# 2+
ax.hlines(khanmm[0].n, xmin=-0.5, xmax=2.5, ls="dashed", lw=1.25, color="crimson")
ax.add_patch(mpl.patches.Rectangle((-0.5, khanmm[0].n - khanmm[0].s), 3, khanmm[0].s * 2, color="red", alpha=0.25, label=r"$2^{+}_{1}$ E. Khan"))
# 3-
ax.hlines(ppmm[3].n, xmin=2.5, xmax=5.5, ls="dashed", lw=1.25, color="darkorange")
ax.add_patch(mpl.patches.Rectangle((2.5, ppmm[3].n - ppmm[3].s), 5.5, ppmm[3].s * 2, color="orange", alpha=0.25, label=r"$3^{-}_{1}$ E.Khan"))

##(d,d) Betas
plot(axs[1][0], ddlabels[:2], ddem[:2], r"EM I. Zanon et al")
plot(axs[1][0], ddlabels[-1], ddem[-1], r"Theo EM E. Tryggestad et al")
plot(axs[1][0], ddlabels, ddnuclear, r"Our exp. (d,d)")

## (d,d) MnMp
plot(axs[1][1], ddlabels, ddmm, r"Simple Bernstein")
plot(axs[1][1], ddlabels, ddgen, r"Generalized Bernstein")
plot(axs[1][1], ddlabels[0], khanmm[0], r"E. Khan", color="red")
plot(axs[1][1], ddlabels[2], ppmm[3], None,color="red")
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
# Custom axis
for ax in [axs[0][0], axs[0][1]]:
    ax.tick_params(axis="x", labelsize=14)

for ax in axs.flatten():
    ax.tick_params(axis="x", which="minor", bottom=False, top=False)

axs[0][0].axvline(2.5, lw=2, ls="dashed", marker="none", color="black")
axs[0][1].axvline(2.5, lw=2, ls="dashed", marker="none", color="black")
# axs[0][1].set_xlim(-0.5, 2.5)

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
# plt.savefig("./Pictures/beta_study.png", dpi=200)
plt.show()
