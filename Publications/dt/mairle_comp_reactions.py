from collections import defaultdict
from re import A
from typing import Dict, List, Tuple

from scipy.fft import dst
import pyphysics as phys
import pickle
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty

# 18O(d,t) results
dt = [
    (11.082, 0.96),
    (12.471, 0.24),
    (12.950, 0.19),
    (13.640, 0.29),
    (16.58, 0.93),
    (18.14, 0.17),
]

d3he = [
    (0, 2.02),
    (1.37, 0.38),
    (1.86, 0.41),
    (2.52, 0.53),
    # (3.20, 0.05),
    # (4, 0.04),
    # (5.17, 0.08),
    (5.523, 1.83),
    (6.99, 0.32),
]

offset = dt[0][0]

# 18O(d,t) transformed
dt_offset = [(ex - offset, sf) for ex, sf in dt]

# Division
div = [(ex, d3he[i][1] / tlow) for i, (ex, tlow) in enumerate(dt_offset)]

# Plot
fig, axs = plt.subplots(1, 2, figsize=(8, 4))
ax: mplaxes.Axes = axs[0]
ax.plot(*zip(*dt_offset), marker="s", label="(d,t)")
ax.plot(*zip(*d3he), marker="o", label="(d,3he)")
ax.set_xlabel(r"E$_{x}$ [MeV]")
ax.set_ylabel("SF")
ax.legend()

ax = axs[1]
ax.plot(*zip(*div), marker="s")
ax.set_xlabel(r"E$_{x}$ [MeV]")
ax.set_ylabel("(d,3He) / (d,t)")
ax.axhline(3 / 2, color="crimson", ls="--", label="ratio = 3/2")
ax.axhline(3, color="dodgerblue", ls="--", label="ratio = 3")
ax.legend()

fig.suptitle("Mairle SF comparison")
fig.tight_layout()
fig.savefig("./Outputs/mairle_comparison_reactions.pdf", dpi=300)
plt.show()
