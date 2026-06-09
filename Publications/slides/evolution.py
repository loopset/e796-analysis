import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import uncertainties as un
import numpy as np
import pickle
import sys

sys.path.append("../")
sys.path.append("/media/Data/E796v2/Publications/dt/")

import styling as sty
import dt

with open("../dt/Inputs/evolution.pkl", "rb") as f:
    stes, cents, gaps = pickle.load(f)

# Binding energy
sn20 = phys.Particle("20O").get_sn()

qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
nuclei = [r"$^{16}$O", r"$^{18}$O", r"$^{20}$O"]

fig, ax = plt.subplots(1, 1, figsize=(5, 3), constrained_layout=True)

# linestyles
ls = ["-", ":", "--"]

# ESPEs
for i, q in enumerate([qp12, qp32]):
    # Datasets
    for j in range(3):
        y = [(un.nominal_value(-d[q]) - sn20) for d in cents[j::3]]
        uy = [un.std_dev(d[q]) for d in cents[j::3]]
        # Exp
        if j == 0:
            ax.errorbar(
                nuclei,
                y,
                yerr=uy,
                color=sty.barplot[q]["ec"],
                **sty.errorbar_nols,
                ls=ls[j],
            )
        else:
            for k in range(len(nuclei)):
                x = [k - 0.2, k + 0.2]
                ax.plot(x, [y[k]] * 2, color=sty.barplot[q]["ec"], ls=ls[j], lw=1.25)

# Axis settings
ax.set_ylim(-22.5, -5)
ax.set_xlim(-1, 2.5)
ax.tick_params(axis="x", which="both", bottom=False, top=False)
ax.set_ylabel(r"$\nu$ ESPE [MeV]")

# Annotations
for q in [qp12, qp32]:
    ax.annotate(
        f"{q.format_simple()}",
        xy=(-0.5, un.nominal_value(-cents[0][q] - sn20)),
        **sty.ann,
        color=sty.barplot[q]["ec"],
    )
ax.annotate(
    "N = 6",
    xy=(-0.5, -un.nominal_value(cents[0][qp12] + cents[0][qp32]) / 2 - sn20),
    **sty.ann,
)

# Fake legend
color = "black"
ax.legend(
    handles=[
        Line2D([], [], color=color, ls="-", label="Exp"),
        Line2D([], [], color=color, ls=":", label="SFO-tls"),
        Line2D([], [], color=color, ls="--", label="Mod1"),
    ],
    loc="upper right",
    fontsize=12,
)

fig.savefig("./Outputs/evolution.png", dpi=300)
plt.show()
