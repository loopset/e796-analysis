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
# Unmodified SFO-tls
plain = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
    ]
)
# Mod SFO-tls
sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)
for model in [plain, sfo]:
    model.set_max_Ex(25)
    model.set_min_SF(0.09)

fig, ax = plt.subplots(1, 1)

# Parameters for each model
nmodels = 3
width = 0.6
left_padding = 0.075
right_padding = 0.075

for i, data in enumerate([exp, plain.data, sfo.data]):
    for q, vals in data.items():
        if q == phys.QuantumNumbers.from_str("0d3/2"):
            continue
        for j, val in enumerate(vals):
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
                label=("" if i == 0 and j == 0 else "_") + q.format(),
            )
            ## Annotate C2S
            ax.annotate(
                f"{sf:.2f}",
                xy=(left - left_padding, ex),
                ha="center",
                va="center",
                fontsize=10,
            )
            ## Annotate Jpi
            pi = "+" if q.l != 1 else "-"
            ax.annotate(
                f"${q.get_j_fraction()}^{{{pi}}}_{{{j}}}$",
                xy=(left + width + right_padding, ex),
                ha="center",
                va="center",
                fontsize=10,
            )

ax.legend(loc="upper left", bbox_to_anchor=(0.05, 0.8, 1, 0.2), ncols=4)
ax.set_xticks([i + 0.5 for i in range(nmodels)], ["Exp", "SFO-tls", "Mod\nSFO-tls"])
ax.tick_params(axis="x", which="both", bottom=False, top=False, pad=15)
ax.tick_params(axis="y", which="both", right=False)
for spine in ["bottom", "top", "right"]:
    ax.spines[spine].set_visible(False)
ax.set_xlim(0, nmodels)
ax.set_ylim(-0.25, 28.5)
ax.set_ylabel(r"E$_{\text{x}}$ [MeV]")

fig.tight_layout()
fig.savefig("./Outputs/vertical.pdf")
plt.show()
