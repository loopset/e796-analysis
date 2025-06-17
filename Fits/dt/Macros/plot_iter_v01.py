import pyphysics as phys
from pyphysics.actroot_interface import SFInterface, FitInterface
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes

g0s = np.arange(0, 1.4, 0.2)
g1s = np.arange(0, 1.4, 0.2)


sfs = np.zeros((len(g0s), len(g1s), 2), dtype=object)
chis = np.zeros((len(g0s), len(g1s)), dtype=float)
for i, g0 in enumerate(g0s):
    for j, g1 in enumerate(g1s):
        # SFs for the two states
        file = f"sfs_{g0:.2f}_{g1:.2f}"
        file_rebin = file + "_rebin"
        for k, f in enumerate([file, file_rebin]):
            inter = SFInterface("./Outputs/Iter/" + f + ".root")
            if k == 0:
                model = inter.get_model("v0", "l = 1")
            else:
                model = inter.get_model("v1", "l = 1")
            if model is not None:
                sfs[i, j, k] = model.fSF
        # Chi2
        file_fit = f"fit_{g0:.2f}_{g1:.2f}"
        fit = FitInterface("./Outputs/Iter/" + file_fit + ".root")
        chis[i, j] = fit.fChi / fit.fNdof
## Create mesh to plot
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
pcm = ax.contourf(X, Y, chis, cmap="managua_r")
fig.colorbar(pcm, ax=ax, label=r"$\chi^{2}_{\nu}$")

fig.tight_layout()
fig.savefig("./Outputs/iter_v01.png")
plt.show()
