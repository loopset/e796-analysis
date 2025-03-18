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


def BEL_to_beta(
    be: float | un.UFloat, z: int, a: int, l: float, up: bool = True
) -> float | un.UFloat:
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


p = Particle(8, 20)
print(p)


def bernstein(
    em: float | un.UFloat, nucl: float | un.UFloat, nz: float, bpbn: float = 1
) -> float | un.UFloat:
    ratio = bpbn * (nucl / em * (1 + 1.0 / bpbn * nz) - 1)
    return ratio


## 20O settings
a = 20
z = 8
n = a - z

## EM dataset
labels = [r"$2_{1}^{+}$", r"$2_{2}^{+}$", r"$3_{1}^{-}$"]
ls = [2, 2, 3]
em = [un.ufloat(5.9, 0.2), un.ufloat(1.3, 0.2), np.nan]
em = [BEL_to_beta(be, z, a, l, False) for be, l in zip(em, ls)]

## (d,d)
dd = [un.ufloat(0.3197, 0.0073), un.ufloat(0.1819, 0.0051), un.ufloat(0.2440, 0.0069)]
ppkhan = [un.ufloat(0.55, 0.06), np.nan, un.ufloat(0.35, 0.05)]

## Results
mnmp = [bernstein(e, nu, n / z) for e, nu in zip(em, dd)]
mnmpkhan = [un.ufloat(2.35, 0.37), np.nan, np.nan]

# Using extended formula
ext = []
for e, nu in zip(em, dd):
    b = Bernstein(p, nu, e)
    b.print()
    ext.append(b.fMnMp)


# Figure
fig, axs = plt.subplots(1, 2, figsize=(9, 5))
axs[0].errorbar(
    labels,
    unp.nominal_values(dd),
    yerr=unp.std_devs(dd),
    fmt="o",
    label=r"$^{20}$O(d,d) E796",
)
axs[0].errorbar(
    labels,
    unp.nominal_values(ppkhan),
    yerr=unp.std_devs(ppkhan),
    fmt="o",
    label=r"$^{20}$O(p,p) E.Khan",
)
axs[0].errorbar(
    labels,
    unp.nominal_values(em),
    yerr=unp.std_devs(em),
    fmt="o",
    label="EM I.Zanon et al",
)
axs[0].set_ylabel(r"$\beta_{L}$")
axs[0].legend(fontsize=14, frameon=True, fancybox=True, shadow=True)
##################### Second axis
axs[1].errorbar(
    labels, unp.nominal_values(mnmp), yerr=unp.std_devs(mnmp), fmt="o", label="E796"
)
axs[1].errorbar(
    labels,
    unp.nominal_values(mnmpkhan),
    yerr=unp.std_devs(mnmpkhan),
    fmt="o",
    label="E.Khan",
)
axs[1].errorbar(
    labels,
    unp.nominal_values(ext),
    yerr=unp.std_devs(ext),
    fmt="o",
    label=r"E796 generalized formula",
)
axs[1].set_ylabel(r"$M_{n}/M_{p}$")
axs[1].legend(fontsize=14, frameon=True, fancybox=True, shadow=True, loc="lower right")

# Titles
axs[0].set_title(r"$\beta$ comparison", fontsize=16)
axs[1].set_title(r"$M_{n} / M_{p}$ comparison", fontsize=16)

# Axis setting
for ax in axs:
    ax.set_xlabel("State")
    ax.set_xlim(-0.5, len(labels) - 1 + 0.5)

plt.tight_layout()
plt.savefig("./Pictures/beta_study.png", dpi=200)
plt.show()
