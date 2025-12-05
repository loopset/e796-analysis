from collections import defaultdict
from re import S
from typing import List
import pyphysics as phys
import uncertainties as un
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
from matplotlib.patches import Rectangle
import copy

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty

# Experimental data
exp = dt.build_sm()
# Secondly modified SFO-tls
theo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1n.txt",
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1p.txt",
    ]
)
df = pd.read_excel("../../Fits/dt/Inputs/SFO_tls_2/o19-isospin-ok.xlsx")
theo.add_isospin("../../Fits/dt/Inputs/SFO_tls_2/summary_O19_sfotls_modtsp3015.txt", df)


theo.set_max_Ex(15)
theo.set_min_SF(0.075)

# Set allowed isospin
# For experimental, stop at Ex = 10
expt52: phys.SMDataDict = defaultdict(list)
for key, vals in exp.items():
    # T = 3/2
    exp[key] = [val for val in vals if un.nominal_value(val.Ex) < 14.8]
    expt52[key] = [val for val in vals if un.nominal_value(val.Ex) > 14.8]
# And copy with isospin 5/2 and mixture (t == -1)
aux = copy.deepcopy(theo)
aux.set_allowed_isospin(2.5)
theot52 = aux
aux = copy.deepcopy(theo)
aux.set_allowed_isospin(-1)
theotmix = aux
# And set allowed isospin
theo.set_allowed_isospin(1.5)


# Plot
fig, ax = plt.subplots(1, 1, figsize=(7, 6))
# Number of models
nmodels = 2
first_call = True


def plot_bars(models: List[phys.SMDataDict], ax, **kwargs) -> list:
    global first_call
    width = 0.6
    height = 0.25
    left_padding = 0.075
    right_padding = 0.075

    ret = []  # collection of bars
    for i, data in enumerate(models):
        for q, vals in data.items():
            if q == phys.QuantumNumbers.from_str("0d3/2"):
                continue
            for j, val in enumerate(vals):
                ex = un.nominal_value(val.Ex)
                sf = un.nominal_value(val.SF)
                max_sf = q.degeneracy()
                ## Left position of barh
                left = (i + 0.5) - width / 2
                color = sty.barplot.get(q, {}).get("ec")
                ec = color if "hatch" in kwargs else "none"
                label = None
                if first_call and (i == 0 and j == 0):
                    label = q.format()
                ## background bar
                back = ax.barh(
                    ex,
                    left=(i + 0.5) - width / 2,
                    width=width,
                    height=height,
                    color=color,
                    edgecolor=ec,
                    alpha=0.35,
                    **kwargs,
                )
                ## foreground bar
                ratio = sf / max_sf
                fore = ax.barh(
                    ex,
                    left=(i + 0.5) - width / 2,
                    width=ratio * width,
                    height=height,
                    color=color,
                    edgecolor=ec,
                    alpha=0.75,
                    label=label,
                    **kwargs,
                )
                ## Annotate C2S
                textc2s = ax.annotate(
                    f"{sf:.2f}",
                    xy=(left - left_padding, ex),
                    ha="center",
                    va="center",
                    fontsize=10,
                )
                ## Annotate Jpi
                pi = "+" if q.l != 1 else "-"
                textjpi = ax.annotate(
                    f"${q.get_j_fraction()}^{{{pi}}}_{{{j}}}$",
                    xy=(left + width + right_padding, ex),
                    ha="center",
                    va="center",
                    fontsize=10,
                )
                ret.append(back)
                ret.append(fore)
                ret.append(textc2s)
                ret.append(textjpi)
    first_call = False
    return ret


# T = 3/2
barst32 = plot_bars([exp, theo.data], ax=ax)
# T = 5/2
barst52 = plot_bars(
    [expt52, theot52.data],
    ax=ax,
    fill=False,
    hatch="/////",
    lw=1.25,
)
# # Mixture (only for theoretical)
# plot_bars(
#     [theotmix[0].data, theotmix[1].data],
#     ax=ax,
#     fill=False,
#     hatch=r"\\\\",
#     lw=1.25,
# )


# Legends
main_leg = ax.legend(loc="upper left", bbox_to_anchor=(0.05, 0.85, 0.5, 0.2), ncols=2)
# Second with hatch coding
# color = "dimgrey"
# handles_hatch = [
#     Patch(fc=color, ec=color, label="T = 3/2"),
#     Patch(fc="none", ec=color, hatch="////", label="T = 5/2"),
#     # Patch(fc="none", ec=color, hatch=r"\\\\", label="T = mix"),
# ]
# second_leg = ax.legend(
#     handles=handles_hatch, loc="lower left", fontsize=14, bbox_to_anchor=(0.05, 0.6)
# )
# ax.add_artist(main_leg)
# Tick parameters
ax.set_xticks(
    [i + 0.5 for i in range(nmodels)],
    ["Exp", "Mod2\nSFO-tls"],
)
ax.tick_params(axis="x", which="both", bottom=False, top=False, pad=15)
ax.tick_params(axis="y", which="both", right=False)
for spine in ["bottom", "top", "right"]:
    ax.spines[spine].set_visible(False)
ax.set_xlim(0, nmodels)
ax.set_ylim(-0.25, 20)
ax.set_ylabel(r"E$_{\text{x}}$ [MeV]")

fig.tight_layout()
fig.savefig("/media/Data/Docs/SSW/figures/vertical0.png", dpi=300)


# Plot first span
span0 = ax.axhspan(ymin=-0.25, ymax=5.7, color="lightskyblue", alpha=0.15)
fig.savefig("/media/Data/Docs/SSW/figures/vertical1.png", dpi=300)
# Highlight second region
ymin = 5.75
ymax0 = 14.5
ymax1 = 12.0
xs = [0, 1, 1, 2]
span1 = ax.fill_between(
    xs, [ymin] * len(xs), [ymax0, ymax0, ymax1, ymax1], color="orange", alpha=0.15
)
fig.savefig("/media/Data/Docs/SSW/figures/vertical2.png", dpi=300)

span0.set_visible(False)
span1.set_visible(False)

# And plot centroids
# but first increase alpha
alpha = 0.1
for bars in [barst32, barst52]:
    for bar in bars:
        if hasattr(bar, "__iter__"):  # BarContainer
            for patch in bar:
                patch.set_alpha(alpha)
        else:  # Text or other
            try:
                bar.set_alpha(alpha)
            except AttributeError:
                pass

# Experimental
cent12 = un.ufloat(6.8, 0.60)
cent32 = un.ufloat(9.79, 0.26)
for i, cent in enumerate([cent12, cent32]):
    ax.plot(
        [0.2, 0.8],
        [un.nominal_value(cent)] * 2,
        color=sty.barplot[dt.qp12 if i == 0 else dt.qp32]["ec"],
    )
    rect = Rectangle(
        xy=(0.2, un.nominal_value(cent) - un.std_dev(cent)),
        width=0.6,
        height=un.std_dev(cent) * 2,
        color=sty.barplot[dt.qp12 if i == 0 else dt.qp32]["ec"],
        alpha=0.45
    )
    ax.add_patch(rect)

# Theoretical
theocent12 = 6.29
theocent32 = 7.98
ax.hlines(theocent12, xmin=1.2, xmax=1.8, color=sty.barplot[dt.qp12]["ec"], lw=1.5)
ax.hlines(theocent32, xmin=1.2, xmax=1.8, color=sty.barplot[dt.qp32]["ec"], lw=1.5)

# Line joining them
ax.plot([0.8, 1.2], [cent12.n, theocent12], color=sty.barplot[dt.qp12]["ec"], ls="--")
ax.plot([0.8, 1.2], [cent32.n, theocent32], color=sty.barplot[dt.qp32]["ec"], ls="--")

fig.savefig("/media/Data/Docs/SSW/figures/vertical3.png", dpi=300)

plt.show()
