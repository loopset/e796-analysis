from matplotlib import hatch
import uproot
import pyphysics as phys
from pyphysics.actroot_interface import FitInterface, SFInterface
import uncertainties as un
import uncertainties.unumpy as unp
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import pandas as pd
import hist

import sys

sys.path.append("../")
sys.path.append("./")

import styling as sty
import dt
import histos

## Ex comparison
hjuan = uproot.open("./Inputs/Ex_and_fitted_states_Extended_update.root")["htot"].to_hist()  # type: ignore
hjuan.axes[0].label = r"E$_{\text{x}}$ [MeV]"
exdt = uproot.open(
    "../../PostAnalysis/RootFiles/Pipe3/tree_20O_2H_3H_front_juan_RPx.root:Sel_Tree"
).arrays(  # type: ignore
    ["Ex"]
)
# Use same binning as Juan
edges = hjuan.axes[0].edges
bw = abs(edges[0] - edges[1])
xmin = -5
xmax = 40
nbins = int((xmax - xmin) / bw)
hdt = hist.Hist.new.Reg(nbins, xmin, xmax, label=r"E$_{\text{x}}$ [MeV]").Double()
# Scale gs
excut = 1
gsfactor = 0.3
gsbin = hdt.axes[0].index(excut)
hdt.fill(exdt[exdt.Ex >= excut].Ex)
# Scale gs
hgs = hdt.copy()
hgs.reset()
hgs.fill(exdt[exdt.Ex < excut].Ex)
hgs *= gsfactor
# Transform Juan's to (d,t) by a shift
bediff = 14.9  # MeV difference
hdiff = hist.Hist.new.Reg(nbins, xmin, xmax).Double()
values = hjuan.values()
for i, value in enumerate(values):
    center = 0.5 * (edges[i] + edges[i + 1])
    center += bediff
    hdiff.fill(center, weight=value)
# Scaling factor
jfactor = 0.75
hdiff *= jfactor


# Plot
fig, ax = plt.subplots(1, 1, figsize=(6, 4))
ax: mplaxes.Axes
## (d,t)
ax.set_title("$^{20}$O(d,t)")
hgs[:gsbin].plot(ax=ax, label=r"(d,t)", color="dodgerblue", **sty.base1d)  # type: ignore
hdt[gsbin:].plot(ax=ax, color="dodgerblue", **sty.base1d)  # type: ignore
hdiff.plot(ax=ax, color="crimson", label=r"(d,$^3$He)", **sty.base1d)
ax.annotate(rf"gs $\times$ {gsfactor:.1f}", xy=(4.5, 335), fontsize=14, ha="center")
ax.legend()
# ax.annotate(
#     f"Offset = BE(19N) - BE(19O)\n = {bediff:.1f} MeV",
#     xy=(0.45, 0.6),
#     xycoords="axes fraction",
#     fontsize=12,
#     ha="center",
#     va="center"
# )
ax.set_ylabel(f"Counts / {bw * 1e3:.0f} keV")
ax.set_xlim(-3, 40)
# # Print line for states
# df = dt.build_df()
# for state in ["v4", "v5", "v6", "v7"]:
#     ex = df[df["name"] == state]["ex"].iloc[0]
#     ax.axvline(un.nominal_value(ex), 0, 0.75, color="crimson", ls="dashed", lw=1.5)  # type: ignore
ax.axvline(bediff, color="green", ls="--", lw=1.25)
fig.tight_layout()
fig.savefig("/media/Data/Docs/SSW/figures/ias.png", dpi=300)

## Miscellanea
# Read data
fit = FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
sfs = SFInterface(
    "../../Fits/dt/Outputs/rebin_sfs.root"
)  ## rebinned cause t5/2 employ rebinned data

states = ["v7"]

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
    cellText=dftab.values,  # type: ignore
    colLabels=dftab.columns,  # type: ignore
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
# plt.close(fig)

plt.show()
