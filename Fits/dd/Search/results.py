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
isUp = [False, False, True]
em = [un.ufloat(5.9, 0.2), un.ufloat(1.3, 0.2), un.ufloat(1.19e3, 0.10e3)]
em = [BEL_to_beta(be, p.fZ, p.fA, ls[i], isUp[i]) for i, be in enumerate(em)]

## (d,d)
dd = [un.ufloat(0.48, 0.0068), un.ufloat(0.1828, 0.0051), un.ufloat(0.2445, 0.0069)]
ppkhan = [un.ufloat(0.55, 0.06), np.nan, un.ufloat(0.35, 0.05)]

## Results
mnmp = [bernstein(e, nu, n / z) for e, nu in zip(em, dd)]
mnmpkhan = [un.ufloat(2.35, 0.37) * 1.5, np.nan, np.nan]

# Using extended formula
ext = []
for e, nu in zip(em, dd):
    b = Bernstein(p, nu, e)
    b.print()
    ext.append(b.fMnMp)


# Figure
fig, axs = plt.subplots(1, 2, figsize=(9, 6))
axs[0].errorbar(
    labels,
    unp.nominal_values(dd),
    yerr=unp.std_devs(dd),
    fmt="s",
    label=r"$^{20}$O(d,d) E796",
)
axs[0].errorbar(
    labels,
    unp.nominal_values(ppkhan),
    yerr=unp.std_devs(ppkhan),
    fmt="s",
    label=r"$^{20}$O(p,p) E.Khan",
)
axs[0].errorbar(
    labels[:2],
    unp.nominal_values(em[:2]),
    yerr=unp.std_devs(em[:2]),
    fmt="o",
    label="EM I. Zanon et al",
)
axs[0].errorbar(
    labels[2],
    unp.nominal_values(em[2]),
    yerr=unp.std_devs(em[2]),
    fmt="o",
    label="EM N. Nakatsuka et al",
)
axs[0].set_ylabel(r"$\beta_{L}$")
axs[0].legend(fontsize=12, frameon=True, fancybox=True, shadow=True, loc=8, bbox_to_anchor=(0.5, 1.02), borderaxespad=0, ncol=1)
axs[0].annotate(r"B(E $3_{1}^{-} \leftarrow 2_{1}^{+}$)", xy=(1.85, 0.52), xytext=(1.05, 0.52), fontsize=12, ha="center", va="center",
            arrowprops=dict(arrowstyle="-"))
##################### Second axis
axs[1].errorbar(
    labels, unp.nominal_values(mnmp), yerr=unp.std_devs(mnmp), fmt="o", label="E796"
)
axs[1].errorbar(
    labels,
    unp.nominal_values(mnmpkhan),
    yerr=unp.std_devs(mnmpkhan),
    fmt="s",
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
axs[1].legend(fontsize=12, frameon=True, fancybox=True, shadow=True, loc=8, bbox_to_anchor=(0.5, 1.02), borderaxespad=0)

# Titles
# axs[0].set_title(r"$\beta$ comparison", fontsize=16)
# axs[1].set_title(r"$M_{n} / M_{p}$ comparison", fontsize=16)

# Axis setting
for ax in axs:
    ax.set_xlabel("State")
    ax.set_xlim(-0.5, len(labels) - 1 + 0.5)

plt.tight_layout()
plt.savefig("./Pictures/beta_study.png", dpi=200)
plt.show()
