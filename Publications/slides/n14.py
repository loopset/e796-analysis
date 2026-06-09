from collections import defaultdict

import pyphysics as phys
import matplotlib.pyplot as plt
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
from matplotlib.lines import Line2D
import pickle
import pandas as pd
import sys

sys.path.append("../")

import styling as sty
import dt.dt as dt

# Theoretical results
cols = ["x", "0p3/2", "0p1/2", "1s1/2", "0d5/2", "0d3/2"]

# SFO-tls
espeSFOtls = np.array(
    [
        [8, -23.046, -15.541, -4.137, -5.382, 1.038],
        [9, -23.443, -16.443, -3.993, -5.886, 0.596],
        [10, -23.839, -17.345, -3.849, -6.390, 0.154],
        [11, -24.235, -18.247, -3.706, -6.893, -0.288],
        [12, -24.631, -19.149, -3.562, -7.397, -0.731],
        [13, -25.027, -20.051, -3.418, -7.901, -1.173],
        [14, -25.423, -20.953, -3.275, -8.405, -1.615],
        [15, -25.707, -20.562, -4.249, -8.261, -1.738],
        [16, -25.992, -20.172, -5.222, -8.117, -1.861],
    ]
)

# SFO-tls mod
espeSFOtlsmod = np.array(
    [
        [8, -23.046, -15.541, -6.387, -7.632, -1.212],
        [9, -23.568, -16.568, -6.368, -8.240, -1.779],
        [10, -24.089, -17.595, -6.349, -8.848, -2.346],
        [11, -24.610, -18.622, -6.331, -9.456, -2.913],
        [12, -25.131, -19.649, -6.312, -10.064, -3.481],
        [13, -25.652, -20.676, -6.293, -10.672, -4.048],
        [14, -26.173, -21.703, -6.275, -11.280, -4.615],
        [15, -26.582, -21.437, -7.312, -11.261, -4.863],
        [16, -26.992, -21.172, -8.349, -11.242, -5.111],
    ]
)

df_tls = pd.DataFrame(espeSFOtls, columns=cols).astype({"x": int})
df_tls_mod = pd.DataFrame(espeSFOtlsmod, columns=cols).astype({"x": int})
dfs = [df_tls, df_tls_mod]
for df in dfs:
    df["gap"] = df["1s1/2"] - df["0d5/2"]

## 16O
# 16O(d,p) adding
o16_add = phys.ShellModel()
o16_add.data = {
    dt.qd52: [phys.ShellModelData(0, un.ufloat(0.84, 0.04))],
    dt.qs12: [phys.ShellModelData(un.ufloat(0.870749, 0.000020), 0.99)],
}
# 16O(d,t) removal
o16_rem = phys.ShellModel()

## 18O
# Adding 18O(d,p)
o18_add = phys.ShellModel()
o18_add = {
    dt.qd52: [phys.ShellModelData(0, 0.57), phys.ShellModelData(3.153, 0.0006)],
    dt.qs12: [phys.ShellModelData(1.471, 1)],
}
# Removal 18O(d,t)
o18_rem = phys.ShellModel()
with open("../../Fits/dt/Mairle_18Odt/Outputs/reanalysis.pkl", "rb") as f:
    dic = pickle.load(f)
    o18_rem.data = dic

##20O
# Adding 20O(d,p)21O
o20_add = phys.ShellModel()
o20_add.data = {
    dt.qd52: [phys.ShellModelData(0, un.ufloat(0.34, 0.03))],
    dt.qs12: [phys.ShellModelData(un.ufloat(1.213, 0.007), un.ufloat(0.77, 0.09))],
    dt.qp12: [],
    dt.qp32: [],
}
# Removal 20O(d,t)
o20_rem = phys.ShellModel()
o20_rem.data = dt.build_sm()

# Compute experimental gap
adds = [o16_add, o18_add, o20_add]
sn_adds = ["17O", "19O", "21O"]
rems = [o16_rem, o18_rem, o20_rem]
sn_rems = ["16O", "18O", "20O"]
exp_espes = []
exp_gaps = []
for i, (add, rem) in enumerate(zip(adds, rems)):
    b = phys.Barager()
    b.set_adding(add, phys.Particle(sn_adds[i]).get_sn())
    b.set_removal(rem, phys.Particle(sn_rems[i]).get_sn())
    b.do_for([dt.qd52, dt.qs12])
    espes = {}
    espes[dt.qd52] = b.get_ESPE(dt.qd52)
    espes[dt.qs12] = b.get_ESPE(dt.qs12)
    exp_espes.append(espes)
    exp_gaps.append(b.get_gap(dt.qd52, dt.qs12))


nuclei = [r"$^{16}$O", r"$^{18}$O", r"$^{20}$O"]
ns = [8, 10, 12]
ls = ["-", "--"]

fig, ax = plt.subplots(1, 1, figsize=(5, 3.75), constrained_layout=True)
# Gaps
ax.errorbar(
    list(range(8, 14, 2)),
    unp.nominal_values(exp_gaps),
    yerr=unp.std_devs(exp_gaps),
    color="crimson",
    ls="none",
    **sty.errorbar_nols,
)
# Theoretical
for j, theo in enumerate(dfs):
    ax.plot(
        theo.x,
        theo.gap,
        color="crimson",
        lw=1.25,
        ls=ls[j],
    )

# Plot N=14 from M. Stanoiu
o22_gap = un.ufloat(4.11, 0.13)
ax.errorbar(
    14, o22_gap.n, yerr=o22_gap.s, color="crimson", ls="none", **sty.errorbar_nols
)

# Axis format
ax.set_xticks(list(range(8, 18, 2)))
ax.set_xlim(5, 18)
ax.set_xlabel("Neutron number")
ax.tick_params(axis="x", which="minor", bottom=False, top=False)
ax.set_ylabel(r"$N = 14$ [MeV]")

# # Annotations
# xpos = 6.5
# ypos = [-7, -2.8]
# for i, q in enumerate([dt.qd52, dt.qs12]):
#     ax.annotate(
#         f"{q.format_simple()}",
#         xy=(xpos, ypos[i]),
#         **sty.ann,
#         color=sty.barplot[q]["ec"],
#     )
# ax.annotate(
#     "N = 14",
#     xy=(xpos, sum(ypos) / 2),
#     **sty.ann,
# )

# Fake legend
color = "black"
ax.legend(
    handles=[
        Line2D([], [], color=color, ls="none", marker="s", label="Exp"),
        Line2D([], [], color=color, ls="-", label="SFO-tls"),
        Line2D([], [], color=color, ls="--", label="Mod1"),
    ],
    # loc="lower left",
    fontsize=12,
)
fig.savefig("./Outputs/n14.png", dpi=300)

plt.show()
