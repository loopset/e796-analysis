import pyphysics as phys
from pyphysics.actroot_interface import DataManInterface, TPCInterface
import hist
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter
import matplotlib.ticker as mpltick
from unittest.mock import patch
import ROOT

# # Front
# events = [
#     (155, 51),
#     (155, 107),
#     (155, 529),
#     (155, 1060),
#     (155, 2037),
#     # (155, 2177), (155, 2300), (155, 3076), (155, 4076), (155, 5474),
#     # (155, 6298), (155, 6359), (155, 6972), (155, 7381), (155, 8422),
#     # (155, 8695), (155, 9002), (155, 9128), (155, 9280), (155, 9658),
# ]

# Lateral
events = [
    (155, 58),
    (155, 222),
    (155, 295),
    (155, 426),
    (155, 630),
    (155, 672),
    (155, 803),
    (155, 868),
    (155, 933),
]

dataman = DataManInterface("../../configs/data.conf", "tpc", (155, 156))
ptr = ROOT.ActRoot.TPCData()  # type: ignore
dataman.set_branch_address("TPCData", ptr)


plt.rcParams["axes.labelsize"] = 14
plt.rcParams["xtick.labelsize"] = 12
plt.rcParams["ytick.labelsize"] = 12


xlim, ylim = (0, 128), (0, 128)  # your actual limits
ax_w = 3.0  # axes width in inches
ax_h = ax_w * (ylim[1] - ylim[0]) / (xlim[1] - xlim[0])
l, r, b, t = 0.6, 0.15, 0.5, 0.25  # margins in inches (room for labels)

W, H = l + ax_w + r, b + ax_h + t
fig = plt.figure(figsize=(W, H))


def animate(pair):
    # Event is the tuple (run, event)
    run, entry = pair
    dataman.get_run_entry(run, entry)
    tpc = TPCInterface(ptr)
    # Clean figure
    fig.clear()
    # Create axes
    ax = fig.add_axes([l / W, b / H, ax_w / W, ax_h / H])  # type: ignore
    tpc.plot("xy", False, True, ax=ax, cbar=False)
    # Axis titles
    ax.set_title(f"E796 run {run} entry {entry}", fontsize=12)
    # Limits
    ax.set_xlim(xlim)
    ax.set_ylim(ylim)
    ax.set_aspect("equal")
    # Ticks
    for axis in (ax.xaxis, ax.yaxis):
        axis.set_major_locator(mpltick.MaxNLocator(nbins=4))
        axis.set_minor_locator(mpltick.AutoMinorLocator(n=2))


anim = FuncAnimation(fig, animate, frames=events)  # type: ignore
with patch("matplotlib.pyplot.sca"):
    anim.save("./e796_lateral.gif", writer=PillowWriter(fps=2), dpi=200)
