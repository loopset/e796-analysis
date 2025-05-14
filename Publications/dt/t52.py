import pyphysics as phys
import uncertainties as un
import uncertainties.unumpy as unp
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import pandas as pd

import sys

sys.path.append("../")
import styling as sty

# Read data
fit = phys.FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
sfs = phys.SFInterface(
    "../../Fits/dt/Outputs/rebin_sfs.root"
)  ## rebinned cause t5/2 employ rebinned data

states = ["v4", "v5", "v6", "v7"]

# Built dataset
ex = [fit.get(state)[0] for state in states]
which = "l = 1"
sf = []
chi2 = []
model = []
for state in states:
    best = sfs.get_model(state, which)
    # best = sfs.get_best(state)
    if best is None:
        continue
    sf.append(best.fSF)
    chi2.append(best.fChi)
    model.append(best.fName)
# df = pd.DataFrame({"ex": ex, "sf": sf, "model": model, "chi2red": chi2})
df = pd.DataFrame({"ex": ex, "sf": sf, "model": model, "chi2": chi2})
dftab = df.copy()
for column in dftab.columns:
    first = dftab[column][0]
    if isinstance(first, un.UFloat):
        dftab[column] = dftab[column].apply(lambda x: f"{x:.2uS}")
    if isinstance(first, float):
        dftab[column] = dftab[column].apply(lambda x: f"{x:.2f}")

# Juan data
dfj = pd.DataFrame(
    {
        "ex": [
            un.ufloat(-0.03, 0.04),
            un.ufloat(1.35, 0.09),
            un.ufloat(2.87, 0.08),
            un.ufloat(5.22, 0.04),
        ],
        "sf": [
            un.ufloat(1.74, 0.11),
            un.ufloat(0.68, 0.07),
            un.ufloat(0.52, 0.05),
            un.ufloat(1.68, 0.09),
        ],
    }
)
## Append Ex relative to that of our gs
dfj["ex"] = dfj["ex"] + df["ex"][0]
# Equivalence factor
isospinf = 5

fig, axs = plt.subplots(1, 2, figsize=(11, 5))
# Table
ax: mplaxes.Axes = axs[0]
ax.axis("off")
ax.table(
    cellText=dftab.values, #type: ignore
    colLabels=dftab.columns, #type: ignore
    cellLoc="center",
    colWidths=[0.2] * len(dftab.columns),
    loc="center",
    fontsize=32,
)
ax.annotate("(d,t) exp", xy=(0.45, 0.65), fontsize=16)
# SFs
ax = axs[1]
ax.errorbar(
    unp.nominal_values(df.ex),
    unp.nominal_values(df.sf),
    xerr=unp.std_devs(df.ex),
    yerr=unp.std_devs(df.sf),
    label="(d,t)",
    **sty.errorbar,
)
ax.errorbar(
    unp.nominal_values(df.ex),
    unp.nominal_values(df.sf * isospinf),
    xerr=unp.std_devs(df.ex),
    yerr=unp.std_devs(df.sf * isospinf),
    ls="none",
    marker="o",
    label=r"(d,t) $\times$ 5",
    capsize=3,
)
ax.errorbar(
    unp.nominal_values(dfj["ex"]),
    unp.nominal_values(dfj["sf"]),
    xerr=unp.std_devs(dfj["ex"]),
    yerr=unp.std_devs(dfj["sf"]),
    label=r"(d,$^{3}$He)",
    **sty.errorbar,
)
ax.legend()
ax.set_xlabel(r"E$_{\mathrm{x}}$ [MeV]")
ax.set_ylabel(r"C$^{2}$S")

fig.tight_layout()
fig.savefig("./Outputs/d_3he_isospin.pdf")
plt.show()
