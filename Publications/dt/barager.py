from cProfile import label
from typing import List, Dict, Union

import matplotlib as mpl
import pyphysics as phys
from pyphysics.actroot_interface import FitInterface, SFInterface

import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as unc
import uncertainties.unumpy as unp
import numpy as np
import ROOT as r

import sys

sys.path.append("./")
sys.path.append("../")

import styling as sty
import dt

# Experimental dt data: removing reaction
exp = dt.build_sm()
low, up = dt.split_isospin(exp)
# split isospin: T = 3/2 -> low

# Adding reactions: B. Fernández-Domínguez PRC 84 (2011)
add = phys.ShellModel()
add.data = {
    dt.qd52: [phys.ShellModelData(0, unc.ufloat(0.34, 0.03))],
    dt.qs12: [phys.ShellModelData(unc.ufloat(1.213, 0.007), unc.ufloat(0.77, 0.09))],
    dt.qp12: [],
    dt.qp32: [],
}

# Path to theoretical files
path = "../../Fits/dt/"
# YSOX
ysox = phys.ShellModel(
    [
        path + "Inputs/SM/log_O20_O19_ysox_tr_j0p_m1p.txt",
        path + "Inputs/SM/log_O20_O19_ysox_tr_j0p_m1n.txt",
    ]
)

# SFO-tls
sfotls = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
    ]
)
# Modified SFO-tls
mod_sfotls = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
    ]
)
# Modify all sms applying the same cuts
for sm in [ysox, sfotls, mod_sfotls]:
    sm.set_max_Ex(10)
    sm.set_min_SF(0.09)

# Binding energies
snadd = r.ActPhysics.Particle("21O").GetSn()  # type: ignore
snrem = r.ActPhysics.Particle("20O").GetSn()  # type: ignore

# Run Barager's formula
labels = ["Exp", "YSOX", "SFO-tls", "Modified\nSFO-tls"]
removals = [low, ysox, sfotls, mod_sfotls]
bars: List[phys.Barager] = []
for i, removal in enumerate(removals):
    b = phys.Barager()
    b.set_removal(removal, snrem)
    b.set_adding(add, snadd)
    b.do_for([dt.qd52, dt.qs12, dt.qp12, dt.qp32])
    bars.append(b)

# Write to file
bars[0].write("./Inputs/dt_barager.pkl")


# Compute centroids
def get_centroids(
    data: phys.SMDataDict,
) -> Dict[phys.QuantumNumbers, Union[float, unc.UFloat]]:
    zero = 0
    ret = {}
    for q, vals in data.items():
        num = 0
        den = 0
        for val in vals:
            num += (2 * q.j + 1) * val.SF * (val.Ex - zero)  # type: ignore
            den += (2 * q.j + 1) * val.SF  # type: ignore
        try:
            ret[q] = num / den
        except ZeroDivisionError:
            ret[q] = np.nan
    return ret


centroids: List[Dict[phys.QuantumNumbers, Union[float, unc.UFloat]]] = []
for rem in removals:
    data = rem
    if isinstance(rem, phys.ShellModel):
        data = rem.data
    centroids.append(get_centroids(data))

# Plotting
fig, axs = plt.subplots(1, 2, figsize=(9, 5))
# ESPES
ax: mplaxes.Axes = axs[0]
for q in [dt.qd52, dt.qs12, dt.qp12, dt.qp32]:
    y = []
    ey = []
    for bar in bars:
        espe = bar.get_ESPE(q)
        y.append(unc.nominal_value(espe))
        ey.append(unc.std_dev(espe))
    ax.errorbar(
        labels,
        y,
        yerr=ey,
        color=sty.barplot.get(q, {}).get("ec"),
        marker="s",
        markersize=5,
        label=q.format(),
    )
ax.legend(loc="lower left", bbox_to_anchor=(0, 1.01, 1, 0.075), ncols=2, fontsize=12)
ax.set_ylabel("ESPE [MeV]")

# Gaps
ax = axs[1]

gap_labels = ["N = 8", "S.O splitting"]
for i in range(2):
    y = []
    ey = []
    for j, bar in enumerate(bars):
        if i == 0:
            gap = bar.get_gap(dt.qd52, dt.qp12)
        else:
            gap = bar.get_gap(dt.qp12, dt.qp32)
        y.append(unc.nominal_value(gap))
        ey.append(unc.std_dev(gap))
    ax.errorbar(labels, y, yerr=ey, marker="o", markersize=5, label=gap_labels[i])

# Draw also centroids
for i in range(2):
    y = []
    ey = []
    for j, cent in enumerate(centroids):
        if i == 0:
            gap = cent.get(dt.qd52) - cent.get(dt.qp12)  # type: ignore
        else:
            gap = cent.get(dt.qp12) - cent.get(dt.qp32)  # type: ignore
        y.append(abs(unc.nominal_value(gap)))
        ey.append(unc.std_dev(gap))
    ax.errorbar(labels, y, yerr=ey, marker="s", markersize=5, ls="--", alpha=0.75)

ax.set_ylabel("Gap [MeV]")
ax.annotate(
    "Dashed: using centroids\ninstead of ESPEs",
    xy=(0.5, 0.75),
    xycoords="axes fraction",
    fontsize=12,
    ha="center",
    va="center",
)
# Draw Juan's results
d3He_exp = unc.ufloat(5.30, 0.10)
d3He_sfo = 5
ax.fill_betweenx(
    [d3He_exp.n - d3He_exp.s, d3He_exp.n + d3He_exp.s],
    x1=-0.5,
    x2=0.5,
    color="turquoise",
    alpha=0.75,
    label="Exp (d,$^{3}$He)",
)
ax.hlines(
    d3He_sfo,
    xmin=1.5,
    xmax=3.5,
    color="turquoise",
    alpha=0.75,
    label="SFO-tls (d,$^{3}$He)",
)
ax.legend(loc="lower left", bbox_to_anchor=(0, 1.01, 1, 0.075), ncols=2, fontsize=12)

fig.tight_layout()
fig.savefig("./Outputs/gap.pdf")
plt.show()
