from collections import defaultdict
from re import S
from typing import List
import pyphysics as phys
import uncertainties as un
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
from matplotlib.patches import Patch
import copy

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty

# Experimental data
exp = dt.build_sm()
# Unmodified SFO-tls
plain = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
    ]
)

# Add isospin
plain.add_isospin("../../Fits/dt/Inputs/SM/summary_O19_psdmk2_sfotls.txt")
# Mod SFO-tls
sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)
df = pd.read_excel("../../Fits/dt/Inputs/SM_fited/o19-isospin-ok.xlsx")
sfo.add_isospin("../../Fits/dt/Inputs/SM_fited/summary_O19_sfotls_mod.txt", df)

## Secondly modified SFO-tls
sfo2 = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1n.txt",
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1p.txt",
    ]
)
df2 = pd.read_excel("../../Fits/dt/Inputs/SFO_tls_2/o19-isospin-ok.xlsx")
sfo2.add_isospin("../../Fits/dt/Inputs/SFO_tls_2/summary_O19_sfotls_modtsp3015.txt", df2)

for model in [plain, sfo, sfo2]:
    model.set_max_Ex(25)
    model.set_min_SF(0.09)

# Set allowed isospin
# For experimental, stop at Ex = 10
expt52: phys.SMDataDict = defaultdict(list)
for key, vals in exp.items():
    # T = 3/2
    exp[key] = [val for val in vals if un.nominal_value(val.Ex) < 14.5]
    expt52[key] = [val for val in vals if un.nominal_value(val.Ex) > 14.5]
# And copy with isospin 5/2 and mixture (t == -1)
theot52, theotmix = [], []
for theo in [plain, sfo, sfo2]:
    aux = copy.deepcopy(theo)
    aux.set_allowed_isospin(2.5)
    theot52.append(aux)
    aux = copy.deepcopy(theo)
    aux.set_allowed_isospin(-1)
    theotmix.append(aux)
    # And set allowed isospin
    theo.set_allowed_isospin(1.5)

# Delete some states messing the plot in plain SFO-tls
plain.data[phys.QuantumNumbers(0, 1, 1.5)].pop(4)
plain.data[phys.QuantumNumbers(0, 1, 1.5)].pop()

fig, ax = plt.subplots(1, 1)

# Number of models
nmodels = 4

first_call = True


def plot_bars(models: List[phys.SMDataDict], ax, **kwargs) -> None:
    global first_call
    width = 0.6
    height = 0.25
    left_padding = 0.075
    right_padding = 0.075

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
                ax.barh(
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
                ax.barh(
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
                ax.annotate(
                    f"{sf:.2f}",
                    xy=(left - left_padding, ex),
                    ha="center",
                    va="center",
                    fontsize=10,
                )
                ## Annotate Jpi
                pi = "+" if q.l != 1 else "-"
                ax.annotate(
                    f"${q.get_j_fraction()}^{{{pi}}}_{{{j}}}$",
                    xy=(left + width + right_padding, ex),
                    ha="center",
                    va="center",
                    fontsize=10,
                )
    first_call = False
    return None


# T = 3/2
plot_bars([exp, plain.data, sfo.data, sfo2.data], ax=ax)
# T = 5/2
plot_bars(
    [expt52, theot52[0].data, theot52[1].data, theot52[2].data],
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
main_leg = ax.legend(loc="upper left", bbox_to_anchor=(0.05, 0.8, 1, 0.2), ncols=4)
# Second with hatch coding
color = "dimgrey"
handles_hatch = [
    Patch(fc=color, ec=color, label="T = 3/2"),
    Patch(fc="none", ec=color, hatch="////", label="T = 5/2"),
    # Patch(fc="none", ec=color, hatch=r"\\\\", label="T = mix"),
]
second_leg = ax.legend(
    handles=handles_hatch, loc="lower left", fontsize=14, bbox_to_anchor=(0.025, 0.6)
)
ax.add_artist(main_leg)
# Tick parameters
ax.set_xticks(
    [i + 0.5 for i in range(nmodels)],
    ["Exp", "SFO-tls", "Mod\nSFO-tls", "Mod2\nSFO-tls"],
)
ax.tick_params(axis="x", which="both", bottom=False, top=False, pad=15)
ax.tick_params(axis="y", which="both", right=False)
for spine in ["bottom", "top", "right"]:
    ax.spines[spine].set_visible(False)
ax.set_xlim(0, nmodels)
ax.set_ylim(-0.25, 28.5)
ax.set_ylabel(r"E$_{\text{x}}$ [MeV]")

fig.tight_layout()
fig.savefig("./Outputs/vertical.pdf")
fig.savefig("./Outputs/vertical.png", dpi=300)

# ## Plot lines separating T states for EuNPC presentation
# ys = [9.7, 15.6, 13.9, 12.65]
# sepx = []
# sepy = []
# width = 0.75
# for i, y in enumerate(ys):
#     sepx.extend([(i + 0.5) - width / 2, (i + 0.5) + width / 2])
#     sepy.extend([y] * 2)
# ax.plot(sepx, sepy, color="purple", marker="none", ls="dashed", lw=1.25, zorder=0)
# fig.tight_layout()
# fig.savefig("./Outputs/vertical_sep.png", dpi=300)

plt.show()
