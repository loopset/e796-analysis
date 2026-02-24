from collections import defaultdict
from re import S
from typing import List, Union
import pyphysics as phys
import uncertainties as un
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
from matplotlib.patches import Patch
import copy
import pickle

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty

############################################### 20O(d,t)
# Experiment
exp = dt.build_sm()
# Mod1 SFO-tls
exp_sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
    ]
)

############################################## 18O(d,t)
# Experiment
with open("../../Fits/dt/Mairle_18Odt/Outputs/reanalysis.pkl", "rb") as f:
    dic = pickle.load(f)
    mairle = phys.ShellModel()
    mairle.data = dic

# Mod2 SFO-tls
with open("../../Fits/dt/Mairle_18Odt/Inputs/SFO-tls/mod1.pkl", "rb") as f:
    mairle_sfo = pickle.load(f)

############################################## 16O(d,t)
# Experiment
with open("../../Fits/dt/16O_dt/exp.pkl", "rb") as f:
    exp16 = pickle.load(f)

# SFO-tls mod2
with open("../../Fits/dt/16O_dt/mod1.pkl", "rb") as f:
    o16_sfo = pickle.load(f)

# Labels
labels = [
    r"$^{16}$O Exp.",
    r"$^{16}$O Mod1",
    r"$^{18}$O Exp.",
    r"$^{18}$O Mod1",
    r"$^{20}$O Exp.",
    r"$^{20}$O Mod1",
]
# Models
models = [exp16, o16_sfo, mairle, mairle_sfo, exp, exp_sfo]
dicts = [getattr(m, "data", m) for m in models]
# Set limits for models
for i, m in enumerate(models):
    if isinstance(m, phys.ShellModel):
        m.set_min_SF(0.04)  # exp threshold
        m.set_max_Ex(15.5)  # same cut as Exp 20O


def compute(models: List[phys.SMDataDict]) -> tuple:
    cents = []
    gaps = []
    for model in models:
        cs = dt.get_centroids(model)
        # Push centroids
        cents.append(cs)
        # Push gaps
        gap = abs(cs[dt.qp12] - cs[dt.qp32])  # type: ignore
        gaps.append(gap)

    return (cents, gaps)


cents, gaps = compute(dicts)


# Vertical plot
fig, ax = plt.subplots()
dt.plot_bars(dicts, labels, ax, height=0.15)
fig.tight_layout()

# Centroids
fig, ax = plt.subplots()
ax: mplaxes.Axes
for counter, i in enumerate(range(0, len(dicts), 2)):
    xs = [counter - 0.4, counter + 0.4]
    for q in [dt.qp12, dt.qp32]:
        color = sty.barplot[q]["ec"]
        for j in range(2):
            y = un.nominal_value(cents[i + j][q])
            uy = un.std_dev(cents[i + j][q])
            ax.plot(xs, [y] * 2, color=color, ls="-" if j == 0 else "--")
            if j == 0:
                ax.fill_between(
                    xs, y1=[y - uy] * 2, y2=[y + uy] * 2, color=color, alpha=0.25
                )

ax.set_xticks([0, 1, 2])
ax.set_xticklabels([r"$^{16}$O", r"$^{18}O$", r"$^{20}$O"])

fig.tight_layout()
plt.show()
