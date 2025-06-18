import pyphysics as phys
from pyphysics.actroot_interface import SFInterface, FitInterface
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import re
import glob
import ROOT as r
r.PyConfig.DisableRootLogon = True #type: ignore

## Read fits
pattern = re.compile(r"fit_(\d*\.\d+)_(\d*\.\d+)\.root")

g0s, g1s = [], []
for filepath in glob.glob("./Outputs/Iter/*.root"):
    filename = filepath.split("/")[-1]
    match = pattern.search(filename)
    if match:
        g0, g1 = float(match.group(1)), float(match.group(2))
        g0s.append(g0)
        g1s.append(g1)

# Create array
g0s = np.array(sorted(g0s))
g1s = np.array(sorted(g1s))

# Path
path = "./Outputs/Iter/"

# Arrays
chis = np.zeros((len(g0s), len(g1s)), dtype=float)
amps = np.zeros((len(g0s), len(g1s)), dtype=float)
sfs = np.zeros((len(g0s), len(g1s), 2), dtype=object)
# if glob.glob(path + "chis.npy"):
if False:
    chis = np.load(path + "chis.npy")
else:
    for i, g0 in enumerate(g0s):
        for j, g1 in enumerate(g1s):
            # Chi
            file_fit = f"fit_{g0:.2f}_{g1:.2f}"
            fit = FitInterface(path + file_fit + ".root", True)
            chis[i, j] = fit.fChi
            amps[i, j] = un.nominal_value(fit.fAmps.get("v2"))

            # # SFs for the two states
            # file = f"sfs_{g0:.2f}_{g1:.2f}"
            # file_rebin = file + "_rebin"
            # if not glob.glob(path + file + ".root"):
            #     continue
            # for k, f in enumerate([file, file_rebin]):
            #     inter = SFInterface(path + f + ".root")
            #     if k == 0:
            #         model = inter.get_model("v0", "l = 1")
            #     else:
            #         model = inter.get_model("v1", "l = 1")
            #     if model is not None:
            #         sfs[i, j, k] = model.fSF
    # Storing them
    np.save(path + "chis.npy", chis)
    np.save(path + "sfs.npy", sfs)

# Create mesh to plot
X, Y = np.meshgrid(g0s, g1s, indexing="ij")

fig, axs = plt.subplots(2, 2, figsize=(12, 6))
vmin = 0
vmax = 1
for i in range(2):
    ax: mplaxes.Axes = axs[0, i]
    pcm = ax.contourf(
        X, Y, unp.nominal_values(sfs[:, :, i]), cmap="managua_r", vmin=vmin, vmax=vmax
    )
    fig.colorbar(pcm, ax=ax, label=f"C$^2$S v{i}")
    ax.set_xlabel(r"$\Gamma_{\text{v0}}$ [MeV]")
    ax.set_ylabel(r"$\Gamma_{\text{v1}}$ [MeV]")
    # Draw current values
    ax.axhline(0.845, color="red", ls="--")
    ax.axvline(0.31, color="red", ls="--")
    ax.plot(0.31, 0.845, marker="o", ms=10, mec="red", mfc="none")

ax = axs[1, 0]
pcm = ax.contourf(X, Y, chis, cmap="managua_r", levels=80)
fig.colorbar(pcm, ax=ax, label=r"$\chi^{2}_{\nu}$")

ax = axs[1, 1]
pcm = ax.contourf(X, Y, amps, cmap="managua_r", levels=40)
fig.colorbar(pcm, ax=ax, label=r"Amp v2")

fig.tight_layout()
fig.savefig("./Outputs/iter_v01.png")
plt.show()
