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
# for sm in [sfo0, sfo1, sfo2]:
#     sm.set_max_Ex(16.5)
#     sm.set_min_SF(0.04)

# Binding energies
snadd = phys.Particle("21O").get_sn()
snrem = phys.Particle("20O").get_sn()

# Run Barager's formula
labels = ["Exp", "SFO-tls", "Mod1", "Mod2"]
removals = [exp, sfo0, sfo1, sfo2]
bars: List[phys.Barager] = []
for i, removal in enumerate(removals):
    b = phys.Barager()
    b.set_removal(removal, snrem)
    b.set_adding(add, snadd)
    b.do_for([dt.qd52, dt.qs12, dt.qp12, dt.qp32])
    bars.append(b)

qs = [dt.qd52, dt.qs12, dt.qp12, dt.qp32]

# Compute centroids
centroids: List[Dict[phys.QuantumNumbers, Union[float, unc.UFloat]]] = []
for rem in removals:
    data = rem
    if isinstance(rem, phys.ShellModel):
        data = rem.data
    centroids.append(dt.get_centroids(data))

# Compute ESPES
espes: List[Dict[phys.QuantumNumbers, Union[float, unc.Variable]]] = []
for bar in bars:
    dic = {}
    for q in qs:
        dic[q] = bar.get_ESPE(q)
    espes.append(dic)

pairs = [(dt.qs12, dt.qd52), (dt.qd52, dt.qp12), (dt.qp12, dt.qp32)]
gaps: List[List[Union[float, unc.Variable]]] = []
# Compute gaps
for bar in bars:
    lis = []
    for top, bottom in pairs:
        gap = bar.get_gap(top, bottom)
        lis.append(gap)
    gaps.append(lis)

fig, axs = plt.subplots(1, 2, figsize=(8, 3.5), constrained_layout=True)
# ESPES
ax: mplaxes.Axes = axs[0]
for q in qs:
    ax.errorbar(
        labels,
        [unc.nominal_value(dic[q]) for dic in espes],
        yerr=[unc.std_dev(dic[q]) for dic in espes],
        color=sty.barplot.get(q, {}).get("ec"),
        marker="s",
        markersize=5,
        label=q.format(),
    )
ax.legend(ncols=2)
ax.set_ylabel("ESPE [MeV]")


# Gaps
ax = axs[1]
gaplabels = ["N = 14", "N = 8", "N = 6"]
gapcolors = ["hotpink", "deepskyblue", "darkorange"]
for i, label in enumerate(gaplabels):
    ax.errorbar(
        labels,
        [unc.nominal_value(lis[i]) for lis in gaps],
        yerr=[unc.std_dev(lis[i]) for lis in gaps],
        label=label,
        color=gapcolors[i],
        **sty.errorbar_line
    )
ax.set_ylabel("Gap [MeV]")
ax.legend()

fig.savefig(sty.thesis + "espes.pdf", dpi=300)
plt.show()
