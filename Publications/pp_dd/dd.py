from typing import Dict
import pyphysics as phys
from pyphysics.actroot_interface import FitInterface
import hist
import uproot

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mpcolor
import matplotlib.lines as mplines
import uncertainties as un

import sys

sys.path.append("../")
import styling as sty

# Kinematic line
kin = uproot.open("../../PostAnalysis/RootFiles/Pipe3/tree_20O_2H_2H_side_juan_RPx.root:Sel_Tree").arrays(["EVertex", "fThetaLight"])  # type: ignore
hkin = (
    hist.Hist.new.Reg(400, 0, 120, label=r"$\theta_{lab}$ [$\circ$]")
    .Reg(600, 0, 15, label=r"$E_{lab}$ [MeV]")
    .Double()
)
hkin.fill(kin["fThetaLight"], kin["EVertex"])

# Fit
fit = FitInterface("../../Fits/dd/Outputs/fit_juan_RPx.root")

# Figure
fig, axs = plt.subplots(1, 2, figsize=(9, 4.5))
# Kin
ax: mplaxes.Axes = axs[0]
hkin.plot(ax=ax, **sty.base2d)
for ex in [0, fit.get("g1")[0]]:
    g = phys.Kinematics(f"20O(d,d)@700|{un.nominal_value(ex)}").get_line3()
    ax.plot(g[0], g[1], lw=1.25)
ax.set_xlim(50, 100)

# Fit
ax = axs[1]
plt.sca(ax)
# ax.set_yscale("log")
fit.plot_hist(**sty.styles["ex"])
fit.format_ax(ax)
fit.plot_global(**sty.styles["global"])


def build_dict_legend(fit: FitInterface, backs: list = []) -> Dict[str, str]:
    ret = {}
    it = iter(backs)
    for key in fit.fFuncs.keys():
        val = ""
        if key == "g0":
            val = "g.s"
        elif "ps" in key:
            val = next(it, "")
        else:
            val = f"{fit.get(key)[0]:.2uS}"
        ret[key] = val
    return ret

labels = build_dict_legend(fit, backs=["1n PS"])
for key in fit.fFuncs.keys():
    fit.plot_func(key, label=labels[key])
ax.legend(loc="upper right", fontsize=12)

# Annotate axis
phys.utils.annotate_subplots(axs)
fig.tight_layout()
fig.savefig(sty.thesis + "20O_dd_fit.pdf", dpi=300)
plt.show()
