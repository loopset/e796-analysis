import pickle
from typing import List, Dict, Union

from matplotlib.lines import Line2D
from matplotlib.patches import Rectangle
import matplotlib.ticker as mpltick

import pyphysics as phys

import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as unc
import copy

import sys

sys.path.append("./")
sys.path.append("../")

import styling as sty
import dt

# Experimental dt data: removing reaction
exp = dt.build_sm()

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

# Not gated
notgated = []
for sm in [sfo0, sfo1, sfo2]:
    notgated.append(copy.deepcopy(sm))

# Modify all sms applying the same cuts
for sm in [sfo0, sfo1, sfo2]:
    sm.set_max_Ex(16.5)
    sm.set_min_SF(0.04)

# Binding energies
snadd = phys.Particle("21O").get_sn()
snrem = phys.Particle("20O").get_sn()

# Adding reactions: B. Fernández-Domínguez PRC 84 (2011)
add = phys.ShellModel()
add.data = {
    dt.qd52: [phys.ShellModelData(0, unc.ufloat(0.34, 0.03))],
    dt.qs12: [phys.ShellModelData(unc.ufloat(1.213, 0.007), unc.ufloat(0.77, 0.09))],
    dt.qp12: [],
    dt.qp32: [],
}
qs = [dt.qd52, dt.qs12, dt.qp12, dt.qp32]


# Function to compute gaps
def compute(data: List[phys.ShellModel]):
    bars: List[phys.Barager] = []
    for i, removal in enumerate(data):
        b = phys.Barager()
        b.set_removal(removal, snrem)
        b.set_adding(add, snadd)
        b.do_for([dt.qd52, dt.qs12, dt.qp12, dt.qp32])
        bars.append(b)

    # Compute centroids
    centroids: List[Dict[phys.QuantumNumbers, Union[float, unc.UFloat]]] = []
    for rem in data:
        aux = rem
        if isinstance(rem, phys.ShellModel):
            aux = rem.data
        centroids.append(dt.get_centroids(aux))  # type: ignore

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

    return (espes, gaps)


# Run calculations
labels = ["Exp", "SFO-tls", "Mod1", "Mod2"]

# With gates
gated = [exp, sfo0, sfo1, sfo2]
espes, gaps = compute(gated)

# NO GATES
notgated.insert(0, exp)
noespes, nogaps = compute(notgated)

# Write ESPES and gaps to disk
with open("./Inputs/espes_gaps.pkl", "wb") as f:
    pickle.dump((espes, gaps), f)
with open("./Inputs/espes_gaps_nogates.pkl", "wb") as f:
    pickle.dump((noespes, nogaps), f)

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
        **sty.errorbar_line,
    )
ax.set_ylabel("Gap [MeV]")
ax.legend()

fig.savefig(sty.thesis + "espes.pdf", dpi=300)

# New figures for gaps
# plt.close("all")
fig, axs = plt.subplots(1, 3, figsize=(8, 2.75), constrained_layout=True)

markers = [None, "s", "o", "*"]
colors = ["green", "dodgerblue", "crimson"]
xs = [None, 0.4, 0.6, None]

# Gated
for i, model in enumerate(gaps):
    for j, gap in enumerate(model[::-1]):
        ax: mplaxes.Axes = axs[j]
        y = unc.nominal_value(gap)
        uy = unc.std_dev(gap)
        x = xs[i]
        marker = markers[i]
        color = colors[j]
        # Experimental
        if i == 0:
            ax.axhline(y, color=color)
            ax.axhspan(ymin=y - uy, ymax=y + uy, color=color, alpha=0.25)
        elif 1 <= i <= 2:
            ax.errorbar(x, y, yerr=uy, marker=marker, color=color)
# Gated
for i, model in enumerate(nogaps):
    for j, gap in enumerate(model[::-1]):
        ax: mplaxes.Axes = axs[j]
        y = unc.nominal_value(gap)
        uy = unc.std_dev(gap)
        x = xs[i]
        marker = markers[i]
        color = colors[j]
        # Experimental
        if i == 0:
            continue
        elif 1 <= i <= 2:
            ax.errorbar(x, y, yerr=uy, marker=marker, ls="--", color=color, mfc=color)

# Custom legend
axs[0].legend(
    handles=[
        Rectangle(xy=(0, 0), width=0, height=0, color="grey", alpha=0.35, label="Exp."),
        Line2D([], [], marker=markers[1], ls="none", color="grey", label="SFO-tls"),
        Line2D([], [], marker=markers[2], ls="none", color="grey", label="Mod1"),
    ],
    handlelength=1.5,
    fontsize=12,
)

# Common settings
labels = [6, 8, 14]
for i, ax in enumerate(axs):
    ax.set_xlim(0.2, 0.8)
    ax.set_xticks([])
    fup = 1.1
    flow = 0.9
    ymin, ymax = ax.get_ylim()
    ax.set_ylim(flow * ymin, fup * ymax)
    ax.annotate(
        rf"$N = {labels[i]}$", xy=(0.5, 0.925), xycoords="axes fraction", **sty.ann
    )
fig.savefig(sty.thesis + "gaps.pdf", dpi=300)
plt.show()
