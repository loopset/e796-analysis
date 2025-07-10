from typing import Dict, List, Tuple, Union
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import pandas as pd
import sys

sys.path.append("./")
sys.path.append("../")
import dt
import styling as sty

## Experimental dataset
exp = dt.build_sm()

## Modified SFO-tls
sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)

## Settings for SFO-tls
# Sum over all states until T = 5/2 arrives
## Ex max determined from vertical.py
sfo.set_max_Ex(13.5)
sfo.set_min_SF(0.05)

## Isospin
# T = 3/2
for key, vals in exp.items():
    vals[:] = [val for val in vals if un.nominal_value(val.Ex) < 10]
df = pd.read_excel("../../Fits/dt/Inputs/SM_fited/o19-isospin-ok.xlsx")
sfo.add_isospin("../../Fits/dt/Inputs/SM_fited/summary_O19_sfotls_mod.txt", df)
sfo.set_allowed_isospin(1.5)


## Strengths
def get_strengths(
    data: phys.SMDataDict,
) -> Dict[phys.QuantumNumbers, Union[float, un.UFloat]]:
    ret = {}
    for q, vals in data.items():
        ret[q] = sum(val.SF for val in data[q])  # type: ignore
    return ret


strens = []
for d in [exp, sfo.data]:
    strens.append(get_strengths(d))


## Centroids
def get_centroids(
    data: phys.SMDataDict,
) -> Dict[phys.QuantumNumbers, Union[float, un.UFloat]]:
    zero = 0
    ret = {}
    for q, vals in data.items():
        num = 0
        den = 0
        for val in vals:
            num += (2 * q.j + 1) * val.SF * (val.Ex - zero)  # type: ignore
            den += (2 * q.j + 1) * val.SF  # type: ignore
        ret[q] = num / den
    return ret


cents = []
for d in [exp, sfo.data]:
    cents.append(get_centroids(d))

# Compute directly gaps, assuming no population in (d,p) reactions
# as Bea's paper proves
gaps = []
for i, d in enumerate(cents):
    gap = d[dt.qp32] - d[dt.qp12]
    gaps.append(gap)
    print(
        "->", "Experimental" if i == 0 else "Mod SFO-tls", " gap : ", f"{gap:.2f} MeV"
    )

## Plot
fig, ax = plt.subplots(figsize=(5, 3.5))
ax: mplaxes.Axes
# Bar settings
width = 0.3  # MeV
for i, (ste, cent) in enumerate(zip(strens, cents)):
    for q, y in ste.items():
        if q == phys.QuantumNumbers(0, 2, 1.5):
            continue
        ax.bar(
            x=un.nominal_value(cent[q]),
            height=un.nominal_value(y),
            yerr=un.std_dev(y) if i == 0 else None,
            width=width,
            align="center",
            label=q.format() if i == 0 else None,
            alpha=1 if i == 0 else 0.5,
            **sty.barplot[q],
        )
ax.legend()
ax.set_ylim(0)
ax.set_xlabel(r"Centroids [MeV]")
ax.set_ylabel(r"$\sum$C$^2$S")
ax.annotate(
    "Dimmed: Mod SFO-tls",
    xy=(0.4, 0.8),
    xycoords="axes fraction",
    ha="center",
    va="center",
    fontsize=14,
)
# ax.annotate(
#     r"$\text{Cent}_{\text{nlj}} = \frac{\text{C}^2\text{S}\cdot \text{E}_\text{x}}{\sum\text{C}^2\text{S}}$",
#     xy=(0.35, 0.75),
#     xycoords="axes fraction",
#     ha="center",
#     va="center",
#     fontsize=18,
# )

fig.tight_layout()
fig.savefig("./Outputs/centroids.png", dpi=300)
plt.show()
