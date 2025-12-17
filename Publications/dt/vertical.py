import copy
import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from matplotlib.patches import Patch
import pandas as pd
import uncertainties as un
from collections import defaultdict
from adjustText import adjust_text

import sys

sys.path.append("./")
sys.path.append("../")

import styling as sty
import dt

# Data
exp = dt.build_sm()

# Theoretical
# Unmodified SFO-tls
sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
    ]
)
sfo.add_isospin("../../Fits/dt/Inputs/SM/summary_O19_psdmk2_sfotls.txt")

## Firstly modified SFO-tls
sfo1 = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)
df1 = pd.read_excel("../../Fits/dt/Inputs/SM_fited/o19-isospin-ok.xlsx")
sfo1.add_isospin("../../Fits/dt/Inputs/SM_fited/summary_O19_sfotls_mod.txt", df1)

## Secondly modified SFO-tls
sfo2 = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1n.txt",
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1p.txt",
    ]
)
df2 = pd.read_excel("../../Fits/dt/Inputs/SFO_tls_2/o19-isospin-ok.xlsx")
sfo2.add_isospin(
    "../../Fits/dt/Inputs/SFO_tls_2/summary_O19_sfotls_modtsp3015.txt", df2
)

# Set isospins
expt52: phys.SMDataDict = defaultdict(list)
exT = 14.8  # MeV, sets experimental separation between T=3/2 and T=5/2
for key, vals in exp.items():
    exp[key] = [val for val in vals if un.nominal_value(val.Ex) < exT]
    expt52[key] = [val for val in vals if un.nominal_value(val.Ex) > exT]

# Settings
theos = [sfo, sfo1, sfo2]
for model in theos:
    model.set_max_Ex(20)
    model.set_min_SF(0.07)


# Clone and set isospin
theos52 = []
for theo in theos:
    clone = copy.deepcopy(theo)
    clone.set_allowed_isospin(2.5)
    theo.set_allowed_isospin(1.5)
    theos52.append(clone)

fig, ax = plt.subplots(figsize=(9, 5.5))
labels = ["Exp", "SFO-tls", "Mod1 SFO-tls", "Mod2 SFO-tls"]
# T = 3/2
t32 = dt.plot_bars([exp, *[m.data for m in theos]], labels, ax=ax)
# T = 5 / 2
t52 = dt.plot_bars(
    [expt52, *[m.data for m in theos52]],
    labels,
    ax=ax,
    first_call=False,
    fill=False,
    hatch="/////",
    # lw=1.25,
)

# Legend
main_leg = ax.legend(loc="upper left", bbox_to_anchor=(0.05, 0.8, 0.5, 0.2), ncols=4)
# Second with hatch coding
color = "dimgrey"
handles_hatch = [
    Patch(fc=color, ec=color, label="T = 3/2"),
    Patch(fc="none", ec=color, hatch="////", label="T = 5/2"),
    # Patch(fc="none", ec=color, hatch=r"\\\\", label="T = mix"),
]
second_leg = ax.legend(
    handles=handles_hatch, loc="lower left", fontsize=14, bbox_to_anchor=(0.05, 0.7)
)
ax.add_artist(main_leg)

# Axis settings
ax.set_ylim(-0.5, 22.5)
ax.set_ylabel(r"$E_{x}$ [MeV]")

fig.tight_layout()
# Adjust texts
texts = t32 + t52
positions = [t.get_position() for t in texts]
adjust_text(
    texts,
    target_x=[p[0] for p in positions],
    target_y=[p[1] for p in positions],
    avoid_self=False,
    only_move=dict(text="y", static="y", explode="y", pull="y"),
    # arrowprops=dict(arrowstyle="-", color="crimson", ls="dotted"),
)

fig.savefig(sty.thesis + "vertical.pdf", dpi=300)
plt.show()
