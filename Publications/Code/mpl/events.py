import pyphysics as phys
from pyphysics.actroot_interface import *
import matplotlib.pyplot as plt
from matplotlib.axes import Axes

import sys

sys.path.append("./")
import styling as sty

import ROOT as r

# Delta electron
delta = TPCInterface(r.TFile("../Events/run_160_entry_49121.root").Get("TPCData"))  # type: ignore

# Multigragmentation
multi = TPCInterface(r.TFile("../Events/run_155_entry_38.root").Get("TPCData"))  # type: ignore

fig, axs = plt.subplots(1, 2, figsize=(6.5, 3), constrained_layout=True)
ax: Axes = axs[0]
delta.plot(ax=ax, cbar=False)
# Annotate delta
ax.annotate(
    r"$\delta e^-$",
    xy=(100, 75),
    xytext=(77, 102),
    **sty.ann,
    arrowprops=sty.arrowprops
)

ax = axs[1]
multi.plot(ax=ax, cbar=False)

# Axis settings
for i, ax in enumerate(axs.flat):
    ax.set_xlabel("")
    ax.set_ylabel("")
    ax.tick_params(
        axis="both", which="both", length=0, labelleft=False, labelbottom=False
    )
    ax.annotate("X", xy=(0.175, 0.0325), xycoords="axes fraction", **sty.ann)
    ax.annotate(
        "",
        xy=(0.15, 0.03),
        xycoords="axes fraction",
        xytext=(0.03, 0.03),
        arrowprops=sty.arrowprops,
    )
    ax.annotate("Y", xy=(0.0325, 0.175), xycoords="axes fraction", **sty.ann)
    ax.annotate(
        "",
        xy=(0.03, 0.15),
        xycoords="axes fraction",
        xytext=(0.03, 0.03),
        arrowprops=sty.arrowprops,
    )

phys.utils.annotate_subplots(axs, x=0.1)
fig.savefig("./Outputs/example_events.pdf", dpi=300)

plt.show()
