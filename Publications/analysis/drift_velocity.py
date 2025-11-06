from typing import List
import pyphysics as phys
from pyphysics.root_interface import parse_tgraph, parse_tf1
import hist
import uproot

import re
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mplcolor

import ROOT as r

import sys

sys.path.append("../")
import styling as sty

chosen = "../../Macros/Drift/Outputs/hs_20O_1H_2H_juan_RPx_drift_2.350.root"
# Read h2
h2d: hist.BaseHist = uproot.open(chosen)["hExZ"].to_hist()  # type: ignore
# Read its fit
file = r.TFile(chosen)  # type: ignore
func = parse_tf1(file.Get("hProfX").GetFunction("pol2"))

# Read all graphs
filegs = r.TFile("./Inputs/drift_corr_funcs.root")  # type: ignore
vdrifts = []
gs = []
g2p = None
g2pfit = None
for key in filegs.GetListOfKeys():
    name = key.GetName()
    if not "fcorr" in name:
        g = filegs.Get(name)
        g2p = parse_tgraph(g)
        g2pfit = parse_tf1(g.GetFunction("pol1"))
        continue
    aux = f"{name};{key.GetCycle()}"
    g = filegs.Get(f"{name};{key.GetCycle()}")
    title = g.GetTitle()
    m = re.search(r"[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?", title)
    if m is not None:
        vdrifts.append(float(m.group(0)))
        gs.append(parse_tgraph(g))

# Read actual
filepd = uproot.open("../../PostAnalysis/RootFiles/Pipe3/tree_20O_1H_2H_front_juan_RPx.root:Sel_Tree").arrays(["Ex", "fSP.fCoordinates.fZ"])  # type: ignore
hpd = (
    hist.Hist.new.Reg(200, -10, 300, label="SP.Z [mm]")
    .Reg(200, -10, 20, label=r"(p,d) E$_{x}$ [MeV]")
    .Double()
)
hpd.fill(filepd["fSP.fCoordinates.fZ"], filepd["Ex"])

## Read p2 parameter


# Plot
fig, axs = plt.subplots(1, 2, figsize=(9, 4.5))
ax: mplaxes.Axes = axs[0]
h2d.plot(ax=ax, **sty.base2d)
ax.plot(func[:, 0], func[:, 1], lw=1.5, color="crimson")
ax.set_xlim(60, 290)
ax.set_ylim(-5, 5)
ax.set_xlabel("")
ax.set_ylabel("")
ax.axhspan(-0.875, 0.875, color="plum", alpha=0.25)

ax = axs[1]
ls = []
for g, vdrift in zip(gs, vdrifts):
    l = ax.plot(g[:, 0], g[:, 1])
    ls.append(l)
# Annotate first and last
# First
ax.annotate(
    r"$v_d = $" + f"{vdrifts[0]} " + r"$mm/\mu s$",
    xy=(0.35, 0.25),
    xycoords="axes fraction",
    **sty.ann,
    color=ls[0][0].get_color(),
)
# Last
ax.annotate(
    r"$v_d = $" + f"{vdrifts[-1]} " + r"$mm/\mu s$",
    xy=(0.35, 0.75),
    xycoords="axes fraction",
    **sty.ann,
    color=ls[-1][0].get_color(),
)
# Arrow
ax.annotate(
    "",
    xy=(0.215, 0.725),
    xytext=(0.215, 0.275),
    xycoords="axes fraction",
    arrowprops=dict(arrowstyle="<-", ls="solid", lw=0.75),
)


ax.set_xlim(60, 290)
ax.set_ylim(-0.6, 0.6)
phys.utils.annotate_subplots(axs)

# Titles
fig.supxlabel(
    "SP.Z [mm]",
    x=0.525,
    y=0.05,
    ha="center",
    va="center",
    fontsize=plt.rcParams["axes.labelsize"],
)
fig.supylabel(r"(p,d) E$_{x}$ [MeV]", fontsize=plt.rcParams["axes.labelsize"])
fig.tight_layout()
fig.subplots_adjust(left=0.085, bottom=0.125)
fig.savefig(sty.thesis + "drift_1.pdf", dpi=300)

############################################ 2nd figure
fig, axs = plt.subplots(1, 2, figsize=(9, 4.5))
ax: mplaxes.Axes = axs[0]
if g2p is not None:
    ax.errorbar(
        g2p[:, 0], g2p[:, 1], yerr=g2p[:, 2], ls="none", marker="o", ms=5, capsize=3
    )
if g2pfit is not None:
    ax.plot(g2pfit[:, 0], g2pfit[:, 1], label="Linear fit")
ax.axhline(0, lw=1.5, ls="--", color="crimson")
# Annotate result
ax.plot(7.72, 0, marker="*", ms=10, color="dodgerblue", mfc="dodgerblue")
ax.annotate(
    r"v$_{d} =$ 7.72(20) mm/$\mu$s",
    xy=(7.72, 0),
    xytext=(7.9, 1.3e-5),
    **sty.ann,
    arrowprops=sty.arrowprops,
)
# And also initial value
ax.annotate(
    "Initial value",
    xy=(7.34, 2.18e-5),
    xytext=(7.7, 3.2e-5),
    **sty.ann,
    arrowprops=sty.arrowprops,
)
ax.set_xlabel(r"v$_{d}$ [mm/$\mu$s]")
ax.set_ylabel(r"p$_{2}$ [MeV/mm$^2$]")
ax.yaxis.set_major_formatter(mpltick.ScalarFormatter(useMathText=True, useOffset=True))
ax.legend(loc="lower left")

ax = axs[1]
hpd.plot(ax=ax, **sty.base2d)
ax.axhline(0, ls="--", color="crimson", lw=1.5)
ax.set_xlim(60, 300)
ax.set_ylim(-5, 5)

phys.utils.annotate_subplots(axs)
axs[0].texts[-1].set_position((0.90, 0.925))

fig.tight_layout()
fig.savefig(sty.thesis + "drift_2.pdf", dpi=300)

plt.show()
