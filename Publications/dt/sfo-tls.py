import pyphysics as phys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties.unumpy as unp
import copy

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty

# Secondly modified SFO-tls
theo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1n.txt",
        "../../Fits/dt/Inputs/SFO_tls_2/log_O20_O19_sfotls_modtsp3015_tr_m0p_m1p.txt",
    ]
)
# Initial settings
# theo.set_max_Ex(12.5)
# theo.set_min_SF(0.07)

exs = np.arange(0, 25, 1)
stres = []
for ex in exs:
    clone = copy.deepcopy(theo)
    clone.set_max_Ex(ex)  # type: ignore
    res = {}
    stre = dt.get_strengths(clone.data)
    for q in [dt.qp12, dt.qp32]:
        res[q] = stre[q]
    stres.append(res)

fig, axs = plt.subplots(1, 1)
# 1 Strength against Ex
ax: mplaxes.Axes = axs
for q in [dt.qp12, dt.qp32]:
    ax.plot(
        exs,
        [unp.nominal_values(dic[q]) / q.degeneracy() * 100 for dic in stres],
        color=sty.barplot[q]["ec"],
        label=q.format(),
    )
ax.set_xlabel(r"E$_{x}$ [MeV]")
ax.set_ylabel("Strength [%]")
ax.set_title("Modification2 SFO-tls")

# Plot our expermental cut
ax.axvline(10, color="crimson", ls="--", label="Up to T = 3/2")
ax.axvline(14.9, color="dodgerblue", label="All states")


ax.legend()
fig.tight_layout()
fig.savefig("./Outputs/sfo-tls_strength.pdf", dpi=300)
plt.show()
