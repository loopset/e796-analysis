import pyphysics as phys
from pyphysics.actroot_interface import FitInterface, SFInterface
import hist
import uproot

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mpcolor
import matplotlib.lines as mplines
import uncertainties as un
import pandas as pd

import sys

sys.path.append("../")
import styling as sty

fit = FitInterface("../../Fits/dd/Outputs/fit_juan_RPx.root", True)
dd = SFInterface("../../Fits/dd/Outputs/sfs.root")
rebin = SFInterface("../../Fits/dd/Outputs/rebin_sfs.root")
states = ["g1", "g2", "g3", "g4", "v0", "v1"]
isRebin = [False, False, False, True, True, True]

# Build df
df = pd.DataFrame(
    {"Ex": [fit.get(state)[0] for state in states]}, index=range(len(states))
).map(lambda x: f"{x:.2uS}")
print(df.T.to_latex())

nrow = 3
ncol = 2

labels = [
    r"$2^{+}_{1}$",
    r"$2^{+}_{2}$",
    r"$3^{-}_{1}$",
    "?",
    r"$3^{-}_{2}$",
    r"$2^{+}_{3}$",
]

colors = sty.cycler.cycler(color=plt.get_cmap("tab10").colors[:4])  # type: ignore
ls = sty.cycler.cycler(ls=["solid", "dashed", "dotted", "dashdot"])

fig, axs = plt.subplots(2, 3, figsize=(10, 4.5), sharex=True, layout="constrained")
for i, state in enumerate(states):
    ax: mplaxes.Axes = axs.flat[i]
    face = rebin if isRebin[i] else dd
    face.plot_exp(state, ax=ax)
    ax.set_prop_cycle(colors + ls)
    face.plot_models(state, ax=ax, lw=0.85)
    face.format_ax(state, ax)
    ex, sigma = fit.get(state)
    ax.annotate(
        f"{ex:.2uS} MeV\n{labels[i]}",
        xy=(0.5, 0.8),
        xycoords="axes fraction",
        **sty.ann,
    )
    if i == 0:
        handles, _ = ax.get_legend_handles_labels()
        ax.legend(
            handles=handles,
            labels=[r"$1^{-}$", r"$2^{+}$", r"$3^{-}$", r"$4^{+}$"],
            loc="center right",
            ncols=1,
            fontsize=12,
            frameon=True,
            fancybox=True,
            labelspacing=0.25,
            columnspacing=1,
        )

for ax in axs[1, :]:
    ax.set_ylim(0, 3)
for ax in axs.flat:
    ax.set_ylabel("")
for ax in axs.flat:
    ax.set_xlabel("")
# Manual labels
axs[1, 1].set_xlabel(r"$\theta_{CM}$ [$\circ$]")
fig.supylabel(
    r"$d\sigma/d\Omega$ [$mb\cdot sr^{-1}$]", fontsize=plt.rcParams["axes.labelsize"]
)

# fig.tight_layout()
fig.savefig(sty.thesis + "20O_dd_ine.pdf", dpi=300)
plt.show()
