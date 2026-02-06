import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import pickle
from typing import List
import sys

sys.path.append("./")
sys.path.append("../")

import styling as sty
import dt

# 16O(d,t) from Purser. Nuclear Physics A132 (1969)
with open("../../Fits/dt/16O_dt/exp.pkl", "rb") as f:
    exp16 = pickle.load(f)

# 18O(d,t) from Mairle et al. REANALYSED
with open("../../Fits/dt/Mairle_18Odt/Outputs/reanalysis.pkl", "rb") as f:
    dic = pickle.load(f)
    mairle = phys.ShellModel()
    mairle.data = dic

# 20O(d,t)
exp = phys.ShellModel()
exp.data = dt.build_sm()

# Labels
labels = [
    "Exp 16O",
    "Reana 18O",
    "Exp 20O",
]
# Models
models: List[phys.ShellModel] = [exp16, mairle, exp]

# Set maximum Ex
for model in models:
    model.set_max_Ex(15.5)

# Build centroids
cents = [dt.get_centroids(d.data) for d in models]
n6 = [cent[dt.qp32] - cent[dt.qp12] for cent in cents]  # type: ignore
for i, n in enumerate(n6):
    print(f"{labels[i]} N=6 gap: {n / n6[0]:.3f}")


fig, ax = plt.subplots()
# Plot
dicts: List[phys.SMDataDict] = [getattr(m, "data", m) for m in models]  # type: ignore
dt.plot_bars(dicts, labels, ax, height=0.15)
fig.tight_layout()

fig, axs = plt.subplots(1, 2)
ax: Axes = axs[0]
ax.errorbar(x=labels, y=unp.nominal_values(n6), yerr=unp.std_devs(n6), marker="o")

fig.tight_layout()
plt.show()
