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
exp = dt.build_sm(dividebyRs=True)

# Theoretical
theos = dt.build_theos(gated=True, c2s_thresh=0.07)

sfo = theos[0]
sfo.add_isospin("../../Fits/dt/Inputs/SM/summary_O19_psdmk2_sfotls.txt")

sfo1 = theos[1]
df1 = pd.read_excel("../../Fits/dt/Inputs/SM_fited/o19-isospin-ok.xlsx")
sfo1.add_isospin("../../Fits/dt/Inputs/SM_fited/summary_O19_sfotls_mod.txt", df1)


# Set isospins
expt52: phys.SMDataDict = defaultdict(list)
exT = 14.8  # MeV, sets experimental separation between T=3/2 and T=5/2
for key, vals in exp.items():
    exp[key] = [val for val in vals if un.nominal_value(val.Ex) < exT]
    expt52[key] = [val for val in vals if un.nominal_value(val.Ex) > exT]

# Settings
theos = [sfo, sfo1]

# Clone and set isospin
theos52 = []
for theo in theos:
    clone = copy.deepcopy(theo)
    clone.set_allowed_isospin(2.5)
    theo.set_allowed_isospin(1.5)
    theos52.append(clone)

# WARNING: tweak to improve visualisation
# For SFO1, delete 3/2- just above IAS
theos[-1].set_max_Ex(exT - 0.25)

fig, ax = plt.subplots(figsize=(5, 5.5))
labels = ["Exp", "SFO-tls", "Mod1"]
# T = 3/2
t32 = dt.plot_bars(
    [exp, *[m.data for m in theos]],
    labels,
    ax=ax,
    width=0.6,
    right_padding=-1,
    left_padding=0.15,
    ann_fontsize=11,
)
# T = 5 / 2
t52 = dt.plot_bars(
    [expt52, *[m.data for m in theos52]],
    labels,
    ax=ax,
    first_call=False,
    fill=False,
    hatch="/////",
    right_padding=-1,
    left_padding=0.15,
    ann_fontsize=11,
    # lw=1.25,
)

# Legend
y_leg = 0.85
main_leg = ax.legend(
    loc="lower left", bbox_to_anchor=(0.015, y_leg, 0.4, 0.15), columnspacing=1, ncols=2
)
# Second with hatch coding
color = "dimgrey"
handles_hatch = [
    Patch(fc=color, ec=color, label="T = 3/2"),
    Patch(fc="none", ec=color, hatch="////", label="T = 5/2"),
    # Patch(fc="none", ec=color, hatch=r"\\\\", label="T = mix"),
]
second_leg = ax.legend(
    handles=handles_hatch,
    loc="lower left",
    fontsize=14,
    bbox_to_anchor=(0.6, y_leg, 0.2, 0.15),
)
ax.add_artist(main_leg)

# Axis settings
ax.set_xlim(-0.175)
ax.set_ylim(-0.5, 20)
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

fig.savefig("../paper/Outputs/vertical.pdf", dpi=300)
plt.show()
