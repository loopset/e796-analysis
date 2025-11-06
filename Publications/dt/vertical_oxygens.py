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
import pickle

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty

############################################### 20O(d,t)
# Experiment
exp = dt.build_sm()
# Mod2 SFO-tls
exp_sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1n.txt",
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1p.txt",
    ]
)
# df2 = pd.read_excel("../../Fits/dt/Inputs/SFO_tls_2/o19-isospin-ok.xlsx")
# exp_sfo.add_isospin(
#     "../../Fits/dt/Inputs/SFO_tls_2/summary_O19_sfotls_modtsp3015.txt", df2
# )

############################################## 18O(d,t)
# Experiment
with open("../../Fits/dt/Mairle_18Odt/Outputs/reanalysis.pkl", "rb") as f:
    dic = pickle.load(f)
    mairle = phys.ShellModel()
    mairle.data = dic

# Mod2 SFO-tls
with open("../../Fits/dt/Mairle_18Odt/Inputs/SFO-tls/mod2.pkl", "rb") as f:
    mairle_sfo = pickle.load(f)

############################################## 16O(d,t)
# Experiment
with open("../../Fits/dt/16O_dt/exp.pkl", "rb") as f:
    exp16 = pickle.load(f)

# SFO-tls mod2
with open("../../Fits/dt/16O_dt/mod2.pkl", "rb") as f:
    o16_sfo = pickle.load(f)

# Labels
labels = [
    "Exp 16O",
    "SFO-tls2 16O",
    "Reana 18O",
    "SFO-tls2 18O",
    "Exp 20O",
    "SFO-tls2 20O",
]
# Models
models = [exp16, o16_sfo, mairle, mairle_sfo, exp, exp_sfo]
# Set limits for models
for i, m in enumerate(models):
    if isinstance(m, phys.ShellModel) and i != 0:
        m.set_min_SF(0.075)
        m.set_max_Ex(18)

dicts = [getattr(m, "data", m) for m in models]

fig, ax = plt.subplots()

# Plot
dt.plot_bars(dicts, labels, ax, height=0.15)

fig.tight_layout()
plt.show()
