import pyphysics as phys
from pyphysics.actroot_interface import TPCInterface
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.patches as mplpatch
import matplotlib.gridspec as mplgrid
import matplotlib.lines as mlines
import numpy as np

import ROOT as r

tpc = r.TFile("./Inputs/run_155_entry_1296.root").Get("TPCData")  # type: ignore
ev = TPCInterface(tpc)

fig = plt.figure(figsize=(5, 5))

# Background axis
bg = fig.add_axes([0, 0, 1, 1], zorder=0)  # type: ignore
bg.set_axis_off()
with_grid = False
if with_grid:
    for v in np.linspace(0, 1, 21):
        bg.axhline(v, color="gray", lw=1, ls="--", zorder=100)
        bg.axvline(v, color="gray", lw=1, ls="--", zorder=100)

# Axis subplots
gs = mplgrid.GridSpec(
    2,
    2,
    height_ratios=[0.15, 0.85],
    width_ratios=[0.15, 0.85],
    left=0.0,
    right=0.96,
    bottom=0.175,
    top=0.96,
    wspace=0.3,
    hspace=0.3,
)

# Top
top = fig.add_subplot(gs[0, 1])
# Left
left = fig.add_subplot(gs[1, 0])
# Right
right = fig.add_subplot(gs[1, 1])
# Axis settings
for ax in [top, left]:
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    for spe in ax.spines.values():
        spe.set_visible(False)

for ax in [top, left, right]:
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_facecolor("none")

# TPC plot
proj = ev.fHist.project("X", "Y")  # type: ignore
vals = proj.values()  # type: ignore
vals[vals <= 0] = np.nan
right.pcolormesh(
    proj.axes[1].edges,  # type: ignore
    proj.axes[0].edges,  # type: ignore
    np.rot90(vals.T, k=-1),  # type: ignore
    cmap="managua_r",
    vmax=4e3,
    rasterized=True,
)
for spe in right.spines.values():
    spe.set_color("dodgerblue")
    spe.set_linewidth(1.75)

# General sil parameters
width = 0.9

# Left silicons
left.plot([0.5] * 2, [0.5 - width / 2, 0.5 + width / 2], color="orchid", lw=4)

# Front silicons
top.plot([0.5 - width / 2, 0.5 + width / 2], [0.5] * 2, color="orange", lw=4)
top.plot([0.5 - width / 2, 0.5 + width / 2], [0.7] * 2, color="gold", lw=4)

# CFA
cfa_x = 0.6
cfa_y = 0.15
cfa_width = 0.05
cfa_height = 0.025
cfa = mplpatch.Rectangle(
    (cfa_x - cfa_width / 2, cfa_y - cfa_height / 2),
    cfa_width,
    cfa_height,
    transform=fig.transFigure,
    lw=1.75,
    color="seagreen",
    fc="none",
)
fig.patches.append(cfa)

# Annotations
fontsize = 16
fontweight = "normal"
fontstyle1 = "normal"
fontstyle2 = "italic"
bg.annotate(
    "",
    xy=(0.35, 0.9),
    xytext=(0.35, 0.76),
    arrowprops=dict(
        arrowstyle="<->", facecolor="black", shrinkA=0.0, shrinkB=0, lw=1.5
    ),
)
bg.annotate(
    "",
    xy=(0.065, 0.65),
    xytext=(0.245, 0.65),
    arrowprops=dict(
        arrowstyle="<->", facecolor="black", shrinkA=0.0, shrinkB=0, lw=1.5
    ),
)
bg.annotate(
    r"d $\sim$ 10 cm",
    xy=(0.175, 0.8),
    fontsize=fontsize,
    fontweight=fontweight,
    fontstyle=fontstyle1,
    ha="center",
    va="center",
)
bg.annotate(
    "Left\nsilicons",
    xy=(0.1, 0.15),
    fontsize=fontsize,
    fontweight=fontweight,
    fontstyle=fontstyle2,
    ha="center",
    va="center",
)
bg.annotate(
    "Front\nsilicons",
    xy=(0.175, 0.925),
    fontsize=fontsize,
    fontweight=fontweight,
    fontstyle=fontstyle2,
    ha="center",
    va="center",
)
bg.annotate(
    r"Beam counter",
    xy=(0.775, cfa_y - cfa_height / 4),
    fontsize=fontsize,
    fontweight=fontweight,
    fontstyle=fontstyle2,
    ha="center",
    va="center",
)
bg.annotate(
    "TPC",
    xy=(0.325, 0.225),
    fontsize=fontsize,
    fontweight=fontweight,
    fontstyle=fontstyle2,
    ha="center",
    va="center",
)
bg.annotate(
    "",
    xy=(0.6, 0.125),
    xytext=(0.6, 0.005),
    arrowprops=dict(
        arrowstyle="->,head_width=0.5, head_length=0.75", color="crimson", lw=2
    ),
)
bg.annotate(
    r"$^{20}$O @ 35 AMeV",
    xy=(0.25, 0.06),
    fontsize=fontsize,
    fontweight=fontweight,
    fontstyle=fontstyle1,
)
bg.annotate(
    r"$2.5 \cdot 10^{4}$ pps",
    xy=(0.65, 0.06),
    fontsize=fontsize,
    fontweight=fontweight,
    fontstyle=fontstyle1,
)

# Save!
plt.savefig("./Outputs/mini_setup.pdf", dpi=300, bbox_inches=None, pad_inches=0.0)
plt.show()
