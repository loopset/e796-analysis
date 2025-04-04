import pyphysics as phys
import numpy as np
import ROOT as r
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as un
import uncertainties.unumpy as unp

# Read experimental xs
with r.TFile("./Outputs/psamp.root") as f:  # type: ignore
    factors = f.Get("Factors")
    mg = f.Get("mgGS")
    exp = {}
    for f, g in zip(factors, mg.GetListOfGraphs()):
        exp[f] = phys.parse_tgraph(g)

# Fit
theo = "/media/Data/E796v2/Fits/pp/Inputs/g0_BG/fort.201"
sfs = {}
for f, data in exp.items():
    c = phys.Comparator(data)
    c.add_model("BG", theo)
    c.fit()
    sfs[f] = c.get_sf("BG")

# Draw
fig, axs = plt.subplots(1, 2, figsize=(9, 4))
ax: mplaxes.Axes = axs[0]
for f, data in exp.items():
    ax.errorbar(data[:, 0], data[:, 1], yerr=data[:, 2], marker="s", label=f"f = {f}")
ax.set_yscale("log")
ax.set_xlabel(r"$\theta_{\mathrm{CM}}$")
ax.set_ylabel("xs [mb/sr]")
ax.legend()

ax: mplaxes.Axes = axs[1]
ax.errorbar(
    *zip(*[(f, unp.nominal_values(sf)) for f, sf in sfs.items()]),
    yerr=[unp.std_devs(sf) for sf in sfs.values()],
    marker="s",
)
ax.set_xlabel("d-breakup scaling")
ax.set_ylabel("g.s SF")

fig.tight_layout()
fig.savefig("./Pictures/psgs.png")
plt.show()
