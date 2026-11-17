import matplotlib

import pyphysics as phys
from pyphysics.actroot_interface import DataManInterface, TPCInterface
import hist
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter
import matplotlib.ticker as mpltick
from unittest.mock import patch
import ROOT
import numpy as np
import matplotlib.gridspec as mplgrid
import matplotlib.patches as mplpatch

events = [(19, 3366), (19, 7841), (19, 45), (19, 97)]  # L1  # L1  # Elastic

dataman = DataManInterface("/media/Data/S2384/configs/data.conf", "tpc", (19, 19))
ptr = ROOT.ActRoot.TPCData()  # type: ignore
dataman.set_branch_address("TPCData", ptr)


plt.rcParams["axes.labelsize"] = 14
plt.rcParams["xtick.labelsize"] = 12
plt.rcParams["ytick.labelsize"] = 12


xlim, ylim = (0, 128), (0, 128)  # your actual limits
fig = plt.figure(figsize=(5, 3))


def animate(pair):
    # Event is the tuple (run, event)
    run, entry = pair
    dataman.get_run_entry(run, entry)
    tpc = TPCInterface(ptr)
    # Clean figure
    fig.clear()

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
        3,
        2,
        figure=fig,
        height_ratios=[0.05, 0.9, 0.05],
        width_ratios=[0.7, 0.3],
        left=0.15,
        right=0.96,
        bottom=0.04,
        top=0.96,
        wspace=0.05,
        hspace=0.2,
    )

    # Top
    top = fig.add_subplot(gs[0, 0])
    # Bottom
    bottom = fig.add_subplot(gs[-1, 0])
    # Centre
    centre = fig.add_subplot(gs[1, 0])
    # Front
    front = fig.add_subplot(gs[1, 1])
    # Axis settings
    for ax in [top, bottom, front]:
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1)
        for spe in ax.spines.values():
            spe.set_visible(False)

    # TPC plot
    tpc.plot(proj="xy", ax=centre, cbar=False)
    for spe in centre.spines.values():
        spe.set_color("dodgerblue")
        spe.set_linewidth(1.75)

    for ax in [top, bottom, centre, front]:
        ax.set_xticks([])
        ax.set_yticks([])
        ax.set_xlabel("")
        ax.set_ylabel("")
        ax.set_facecolor("none")

    # General sil parameters
    width = 0.9

    # Top silicons
    top.plot([0.5 - width / 2, 0.5 + width / 2], [0.5] * 2, color="orchid", lw=4)

    # Bottom silicons
    bottom.plot([0.5 - width / 2, 0.5 + width / 2], [0.5] * 2, color="orchid", lw=4)

    # Front silicons
    front.plot([0.1] * 2, [0.6, 0.95], color="orange", lw=2)
    front.plot([0.1] * 2, [0.05, 0.4], color="orange", lw=2)

    # ZDD
    zdd = mplpatch.Rectangle(
        xy=(0.3, 0.45), width=0.6, height=0.1, color="purple", lw=1.5, fc="none"
    )
    front.add_patch(zdd)

    # Annotations
    fontsize = 10
    fontweight = "normal"
    fontstyle1 = "normal"
    fontstyle2 = "italic"
    # Silicons
    bg.annotate(
        "Auxiliary Si detectors",
        xy=(0.3, 0.9),
        fontsize=fontsize,
        fontweight=fontweight,
        fontstyle=fontstyle2,
        ha="center",
        va="center",
    )
    # Beam
    bg.annotate(
        "",
        xy=(0.005, 0.5),
        xytext=(0.125, 0.5),
        arrowprops=dict(
            arrowstyle="<-,head_width=0.5, head_length=0.75", color="crimson", lw=2
        ),
    )
    bg.annotate(
        r"$\mathbf{^{17}C}$" + "\n15 AMeV" + "\n10$^{3}$ pps",
        xy=(0.065, 0.4),
        fontsize=fontsize,
        fontweight=fontweight,
        fontstyle=fontstyle1,
        ha="center",
        va="center",
    )
    # ACTAR TPC
    bg.annotate(
        "ACTAR TPC",
        xy=(0.35, 0.81),
        fontsize=fontsize,
        fontweight=fontweight,
        fontstyle=fontstyle2,
        color="dodgerblue",
        ha="center",
        va="center",
    )
    bg.annotate(
        r"95% D$_2$ + 5% CF$_4$" + "\n@ 900 mbar",
        xy=(0.55, 0.8),
        fontsize=10,
        fontweight=fontweight,
        fontstyle=fontstyle1,
        color="dodgerblue",
        ha="center",
        va="center",
    )
    # ZDD
    bg.annotate(
        "ZDD",
        xy=(0.875, 0.425),
        fontsize=fontsize,
        fontweight=fontweight,
        fontstyle=fontstyle2,
        color="purple",
        ha="center",
        va="center",
    )

    return


anim = FuncAnimation(fig, animate, frames=events)  # type: ignore
with patch("matplotlib.pyplot.sca"):
    anim.save("./Outputs/pac_slides.gif", writer=PillowWriter(fps=1), dpi=200)
