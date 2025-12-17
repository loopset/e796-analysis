from typing import List, Dict, Union

import pyphysics as phys

import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as unc

import sys

sys.path.append("./")
sys.path.append("../")

import styling as sty
import dt

# Experimental dt data: removing reaction
exp = dt.build_sm()

# Adding reactions: B. Fernández-Domínguez PRC 84 (2011)
add = phys.ShellModel()
add.data = {
    dt.qd52: [phys.ShellModelData(0, unc.ufloat(0.34, 0.03))],
    dt.qs12: [phys.ShellModelData(unc.ufloat(1.213, 0.007), unc.ufloat(0.77, 0.09))],
    dt.qp12: [],
    dt.qp32: [],
}

# SFO-tls
sfo0 = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
    ]
)
# Modified SFO-tls
sfo1 = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
    ]
)
# Modified2 SFO-tls
sfo2 = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1n.txt",
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1p.txt",
    ]
)
# Modify all sms applying the same cuts
for sm in [sfo0, sfo1, sfo2]:
    sm.set_max_Ex(16.5)
    sm.set_min_SF(0.04)

# Binding energies
snadd = phys.Particle("21O").get_sn()
snrem = phys.Particle("20O").get_sn()

# Run Barager's formula
labels = ["Exp", "SFO-tls", "Mod1\nSFO-tls", "Mod2\nSFO-tls"]
removals = [exp, sfo0, sfo1, sfo2]
bars: List[phys.Barager] = []
for i, removal in enumerate(removals):
    b = phys.Barager()
    b.set_removal(removal, snrem)
    b.set_adding(add, snadd)
    b.do_for([dt.qd52, dt.qs12, dt.qp12, dt.qp32])
    bars.append(b)

# Compute centroids
centroids: List[Dict[phys.QuantumNumbers, Union[float, unc.UFloat]]] = []
for rem in removals:
    data = rem
    if isinstance(rem, phys.ShellModel):
        data = rem.data
    centroids.append(dt.get_centroids(data))

# ESPES
fig, ax = plt.subplots(1, 1, figsize=(5.5, 4.25))
ax: mplaxes.Axes
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
ax.legend(ncols=2)
ax.set_ylabel("ESPE [MeV]")
fig.tight_layout()
fig.savefig(sty.thesis + "espes.pdf", dpi=300)

# Gaps
# ax = axs[1]

# gap_labels = ["N = 8", "S.O splitting"]
# for i in range(2):
#     y = []
#     ey = []
#     for j, bar in enumerate(bars):
#         if i == 0:
#             gap = bar.get_gap(dt.qd52, dt.qp12)
#         else:
#             gap = bar.get_gap(dt.qp12, dt.qp32)
#         y.append(unc.nominal_value(gap))
#         ey.append(unc.std_dev(gap))
#     ax.errorbar(labels, y, yerr=ey, marker="o", markersize=5, label=gap_labels[i])

# # Draw also centroids
# for i in range(2):
#     y = []
#     ey = []
#     for j, cent in enumerate(centroids):
#         if i == 0:
#             gap = cent.get(dt.qd52) - cent.get(dt.qp12)  # type: ignore
#         else:
#             gap = cent.get(dt.qp12) - cent.get(dt.qp32)  # type: ignore
#         y.append(abs(unc.nominal_value(gap)))
#         ey.append(unc.std_dev(gap))
#     ax.errorbar(labels, y, yerr=ey, marker="s", markersize=5, ls="--", alpha=0.75)

# ax.set_ylabel("Gap [MeV]")
# ax.annotate(
#     "Dashed: using centroids\ninstead of ESPEs",
#     xy=(0.5, 0.75),
#     xycoords="axes fraction",
#     fontsize=12,
#     ha="center",
#     va="center",
# )
# # Draw Juan's results
# d3He_exp = unc.ufloat(5.30, 0.10)
# d3He_sfo = 5
# ax.fill_betweenx(
#     [d3He_exp.n - d3He_exp.s, d3He_exp.n + d3He_exp.s],
#     x1=-0.5,
#     x2=0.5,
#     color="turquoise",
#     alpha=0.75,
#     label="Exp (d,$^{3}$He)",
# )
# ax.hlines(
#     d3He_sfo,
#     xmin=1.5,
#     xmax=3.5,
#     color="turquoise",
#     alpha=0.75,
#     label="SFO-tls (d,$^{3}$He)",
# )
# ax.legend(loc="lower left", bbox_to_anchor=(0, 1.01, 1, 0.075), ncols=2, fontsize=12)

# fig.savefig("./Outputs/gap.pdf")
plt.show()
