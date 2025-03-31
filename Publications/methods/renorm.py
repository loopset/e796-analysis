import matplotlib as mpl
import matplotlib.axes as mpl_axes
import matplotlib.pyplot as plt
import numpy as np
from cycler import cycler

import pyphysics as phys

# Read experimental cross section
gs = phys.parse_txt("./Inputs/renorm_pp_gs.dat", 3)

# Read theoretical xs
theos = {}
files = {
    "KD": "../../Fits/pp/Inputs/g0_KD/fort.201",
    "CH89": "../../Fits/pp/Inputs/g0_CH89/fort.201",
    "BG": "../../Fits/pp/Inputs/g0_BG/fort.201",
}
for key, file in files.items():
    data = phys.parse_txt(file)
    theos[key] = phys.create_spline3(data[:, 0], data[:, 1])

colors = cycler(color=["#ce5e60", "#59d354", "#5954d8"])

# Create figures
fig, ax = plt.subplots(1, 1, figsize=(4, 3.5))
ax: mpl_axes.Axes
ax.errorbar(
    gs[:, 0], gs[:, 1], yerr=gs[:, 2], marker="s", ls="none", color="black", ms=4
)
ax.set_prop_cycle(colors)
# Draw splines
x = np.linspace(15, 30, 100)
for key, spe in theos.items():
    ax.plot(x, spe(x), lw=2, label=key)
ax.legend(fontsize=10)
# ax.xaxis.set_major_locator(mpl.ticker.MaxNLocator(integer=True))
ax.set_xlabel(r"$\theta_{\mathrm{CM}} [^{\circ}]$")
ax.set_ylabel(r"$d\sigma/d\Omega$ [mb/sr]")
ax.set_ylim(1e2, 1e4)
ax.set_yscale("log")

fig.tight_layout()
fig.savefig("./Outputs/renorm.png", dpi=200)
plt.show()
