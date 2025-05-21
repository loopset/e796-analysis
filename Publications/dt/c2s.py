import copy
from matplotlib import scale
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import pandas as pd
import uncertainties as un
import uncertainties.unumpy as unp

import sys

sys.path.append("./")
sys.path.append("../")
import dt
import styling as sty

# Experimental
df = dt.build_df()
exp = dt.build_sm()

# Transform
sd_sum = exp[dt.qd52][0].SF + exp[dt.qs12][0].SF  # type: ignore
theo_sum = 4  # max occupancy of sd shell
scale = theo_sum / sd_sum
print(f"sd-sum : {sd_sum:.2uS}, scaling : {scale:.2uS}")
exp_trans = copy.deepcopy(exp)
for q, vals in exp_trans.items():
    for val in vals:
        val.SF *= scale  # type: ignore
df["sf_scaled"] = df["sf"] * scale

# SFO-tls
sfo_mod = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)
sfo_mod.set_max_Ex(15)
sfo_mod.set_min_SF(0.075)

# sd sum
labels = ["Experiment", "SFO\n(0d5/2 + 1s1/2)", "SFO\n(0d5/2 + 1s1/2 + 0d3/2)"]
sd_list = [
    sd_sum,
    sum(sfo_mod.sum_strength(q) for q in [dt.qd52, dt.qs12]),  # type: ignore
    sum(sfo_mod.sum_strength(q) for q in [dt.qd52, dt.qs12, phys.QuantumNumbers(0, 2, 1.5)]),  # type: ignore
]

# Scale gs (just for plotting)
for data in [exp, exp_trans, sfo_mod.data]:
    if data.get(dt.qd52) is not None:
        data[dt.qd52][0].SF *= 0.5  # type: ignore

# Draw
fig, axs = plt.subplots(2, 2, figsize=(14, 7))
# Table
ax: mplaxes.Axes = axs[0][0]
dftab = df.copy()
dftab = dftab.drop(columns=["name"], axis=1)
if dftab is None:
    raise ValueError("None in dftab")
for column in dftab.columns:
    first = dftab[column][0]
    if isinstance(first, un.UFloat):
        dftab[column] = dftab[column].apply(lambda x: f"{x:.2uS}")
    if isinstance(first, float):
        dftab[column] = dftab[column].apply(lambda x: f"{x:.2f}")
ax.table(
    cellText=dftab.values,  # type: ignore
    colLabels=dftab.columns,  # type: ignore
    cellLoc="center",
    colWidths=[0.2] * len(dftab.columns),
    loc="center",
    fontsize=24,
)
ax.axis("off")

# sd strength
ax = axs[0, 1]
ax.errorbar(labels, unp.nominal_values(sd_list), yerr=unp.std_devs(sd_list), marker="s")
ax.tick_params(axis="x", labelsize=10)
ax.axhline(4, color="crimson", ls="--")
ax.set_ylabel("sd-shell occupancy")

# Barplot
ax = axs[1][0]
ax.sharey(axs[1, 1])
ax.sharex(axs[1, 1])
for q, vals in exp.items():
    ax.bar(
        unp.nominal_values([e.Ex for e in vals]),
        unp.nominal_values([e.SF for e in vals]),
        width=0.350,
        align="center",
        label=q.format(),
        **sty.barplot.get(q, {}),
    )
for q, vals in exp_trans.items():
    ax.bar(
        unp.nominal_values([e.Ex for e in vals]) + 0.35,
        unp.nominal_values([e.SF for e in vals]),
        width=0.350,
        align="center",
        label="_" + q.format(),
        alpha=0.5,
        **sty.barplot.get(q, {}),
    )
ax.legend()
ax.annotate(
    "Transparent: SF fitted to\nsd-shell max occupancy:\n" + rf"$\mathbf{{SF}} \times \mathbf{{{scale:.2uS}}}$",
    xy=(0.25, 0.75),
    xycoords="axes fraction",
)
ax.set_title("Experimental")

# SFO-tls
ax = axs[1, 1]
for q, vals in sfo_mod.data.items():
    ax.bar(
        unp.nominal_values([e.Ex for e in vals]),
        unp.nominal_values([e.SF for e in vals]),
        width=0.350,
        align="center",
        label=q.format(),
        **sty.barplot.get(q, {}),
    )
ax.legend()
ax.set_title("SFO-tls mod")

for ax in [axs[1, 0], axs[1, 1]]:
    ax.set_xlabel(r"E$_{\mathrm{x}}$ [MeV]")
    ax.set_ylabel(r"C$^2$S")
    ax.annotate(r"gs $\times$ 0.5", xy=(0.5, 2), fontsize=14)

fig.tight_layout(pad=1.5)
fig.savefig("./Outputs/c2s.pdf", dpi=300)
plt.show()
