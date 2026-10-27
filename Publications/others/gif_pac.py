import pyphysics as phys
from pyphysics.actroot_interface import DataManInterface, TPCInterface
import hist
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter
import matplotlib.ticker as mpltick
from unittest.mock import patch
import ROOT

events = [(19, 3366), (19, 7841), (19, 45), (19, 97)]  # L1  # L1  # Elastic

dataman = DataManInterface("/media/Data/S2384/configs/data.conf", "tpc", (19, 19))
ptr = ROOT.ActRoot.TPCData()  # type: ignore
dataman.set_branch_address("TPCData", ptr)


plt.rcParams["axes.labelsize"] = 14
plt.rcParams["xtick.labelsize"] = 12
plt.rcParams["ytick.labelsize"] = 12


xlim, ylim = (0, 128), (0, 128)  # your actual limits
fig = plt.figure(figsize=(5.5, 3.75), constrained_layout=True)


def animate(pair):
    # Event is the tuple (run, event)
    run, entry = pair
    dataman.get_run_entry(run, entry)
    tpc = TPCInterface(ptr)
    # Clean figure
    fig.clear()
    # Create axes
    ax = fig.add_subplot()  # type: ignore
    tpc.plot("xy", False, True, ax=ax, cbar=False)
    # Axis labels
    ax.set_xlabel("")
    ax.set_ylabel("")
    # Limits
    ax.set_xlim(xlim)
    ax.set_ylim(ylim)
    # Ticks
    for axis in (ax.xaxis, ax.yaxis):
        axis.set_major_locator(mpltick.NullLocator())
        axis.set_minor_locator(mpltick.NullLocator())
    # Spines
    ax.set_axis_off()


anim = FuncAnimation(fig, animate, frames=events)  # type: ignore
with patch("matplotlib.pyplot.sca"):
    anim.save("./pac.gif", writer=PillowWriter(fps=1), dpi=200)
