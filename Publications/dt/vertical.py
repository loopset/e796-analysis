from re import S
import pyphysics as phys
import uncertainties as un
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty

# Experimental data
exp = dt.build_sm()
# Mod SFO-tls
sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)
sfo.set_max_Ex(15)
sfo.set_min_SF(0.09)

fig, ax = plt.subplots(1, 1)

# Parameters for each model
nmodels = 2
width = 0.6
left_padding = 0.05

for i, data in enumerate([exp, sfo.data]):
    for q, vals in data.items():
        for val in vals:
            ex = un.nominal_value(val.Ex)
            sf = un.nominal_value(val.SF)
            max_sf = q.degeneracy()
            ## Left position of barh
            left = (i + 0.5) - width / 2
            ## background bar
            ax.barh(
                ex,
                left=(i + 0.5) - width / 2,
                width=width,
                height=0.2,
                color=sty.barplot.get(q, {}).get("ec"),
                alpha=0.35,
            )
            ## foreground bar
            ratio = sf / max_sf
            ax.barh(
                ex,
                left=(i + 0.5) - width / 2,
                width=ratio * width,
                height=0.2,
                color=sty.barplot.get(q, {}).get("ec"),
                alpha=0.75,
            )
            ## Annotate C2S
            ax.annotate(
                f"{sf:.2f}",
                xy=(left - left_padding, ex),
                ha="center",
                va="center",
                fontsize=10,
            )

ax.set_xticks([i + 0.5 for i in range(nmodels)], ["Exp", "Mod\nSFO-tls"])
ax.tick_params(axis="x", which="both", bottom=False, top=False, pad=15)
ax.tick_params(axis="y", which="both", right=False)
for spine in ["bottom", "top", "right"]:
    ax.spines[spine].set_visible(False)
ax.set_xlim(0, nmodels)
ax.set_ylim(-0.25)
ax.set_ylabel(r"E$_{\text{x}}$ [MeV]")

fig.tight_layout()
plt.show()
