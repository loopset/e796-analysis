from re import A
import pyphysics as phys
from pyphysics.root_interface import parse_tcutg, parse_tf1, parse_th1
import hist
import uproot

import numpy as np
import awkward as ak
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mplcolor

import ROOT as r
import cycler
import sys

sys.path.append("../")
import styling as sty

## Read data
data = uproot.open("./Inputs/sp_antiveto.root:SP_Tree").arrays()  # type: ignore

xbins = (300, -20, 290)
ybins = (300, 20, 330)
h = hist.Hist.new.Reg(*xbins, label="Y [mm]").Reg(*ybins, label="Z [mm]").Double()

# Gate on index
idxs = [0, 2, 3, 4, 5, 7, 8, 10]
npcol = ak.to_numpy(data["SilN"])
mask = np.isin(npcol, idxs)
h.fill(data["fCoordinates.fY"][mask], data["fCoordinates.fZ"][mask])

# Parse SM
sm = r.ActPhysics.SilMatrix("AntiVeto")  # type: ignore
sm.Read("/media/Data/E796v2/Macros/SilVetos/Outputs/antiveto_matrix.root")
gs = []
centres = []
for idx in idxs:
    g = sm.GetSil(idx)
    centre = sm.GetCentre(idx)
    gs.append(parse_tcutg(g))
    centres.append((centre.first, centre.second))

## Read projections
ps = []
funs = []
for i, file in enumerate(
    ["./Inputs/antiveto_proj8z.root", "./Inputs/antiveto_proj8y.root"]
):
    f = r.TFile(file)  # type: ignore
    key = "xy" if i == 1 else "z"
    p = f.Get(f"p{key}8_norm")
    ps.append(parse_th1(p))
    fl = p.GetFunction("fleft")
    fr = p.GetFunction("fright")
    funs.append((parse_tf1(fl), parse_tf1(fr)))


fig = plt.figure(figsize=(9, 4.5))
grid = fig.add_gridspec(nrows=2, ncols=2, width_ratios=[1, 1])

# Projection Z
axtop = fig.add_subplot(grid[0, 0])
ps[0].plot(ax=axtop, **sty.base1d)
for f in funs[0]:
    axtop.plot(f[:, 0], f[:, 1], lw=1.25)
axtop.set_xlim(210, 320)

# Projection Y
axbot = fig.add_subplot(grid[1, 0])
ps[1].plot(ax=axbot, **sty.base1d)
for f in funs[1]:
    axbot.plot(f[:, 0], f[:, 1], lw=1.25)
axbot.set_xlabel("Y [mm]")
axbot.set_xlim(140, 260)
fig.supylabel("Normalised counts", x=0.02, fontsize=plt.rcParams["axes.labelsize"])


# SM subplot
axl = fig.add_subplot(grid[:, 1])
ret = h.plot(ax=axl, norm=mplcolor.LogNorm(), **sty.base2d_nocmin)
phys.utils.apply_ROOT_colorbar(ret[1])
for i, g in enumerate(gs):
    axl.plot(g[:, 0], g[:, 1], lw=1.5, color="hotpink")
    axl.annotate(
        f"{idxs[i]}", xy=centres[i], **sty.ann, fontweight="bold", color="white"
    )

for i, ax in enumerate([axtop, axbot, axl]):
    ax.annotate(
        chr(97 + i) + ")",
        xy=(0.125, 0.8 if i <= 1 else 0.925),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=18,
        fontweight="bold",
    )

fig.tight_layout()
fig.savefig(sty.thesis + "antiveto_matrix.pdf", dpi=300)
plt.show()
