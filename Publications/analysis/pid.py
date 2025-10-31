from typing import List
import pyphysics as phys
import hist
import uproot

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mplcolor

import sys

sys.path.append("../")
import styling as sty

# Labels
labels = ["l0", "f0", "f0-f1"]
files = [
    "../pid/Inputs/pid_side.root:PID_Tree",
    "../pid/Inputs/pid_front.root:PID_Tree",
    "../pid/Inputs/pid_two.root:PID_Tree",
]

# Histogram models
ebins = (400, 0, 40)
qbins = (800, 0, 2000)

# Fill histograms
hs: List[hist.BaseHist] = []
for i, file in enumerate(files):
    df = uproot.open(file).arrays(["ESil0", "fQave"] if i == 1 else None)  # type: ignore
    if i < 2:  # one sil plot
        h = (
            hist.Hist.new.Reg(*ebins, label=r"E$_{sil}$ [MeV]")
            .Reg(*qbins, label=r"$\Delta$E$_{gas}$ [arb. unit]")
            .Double()
        )
        h.fill(df["ESil0"], df["fQave"])
    else:  # two sils
        h = (
            hist.Hist.new.Reg(*ebins, label=r"E$_{1}$ [MeV]")
            .Reg(*ebins, label=r"$\Delta$E$_{0}$ [MeV]")
            .Double()
        )
        h.fill(df["ESil1"], df["ESil0"])
    hs.append(h)

# Annotate pids
def annotate(label: str, pos: tuple, l: float, a: float):
    xt = pos[0] + l * np.cos(np.radians(a))
    yt = pos[1] + l * np.sin(np.radians(a))
    ax.annotate(
        label,
        xy=pos,
        xytext=(xt, yt),
        weight="bold",
        **sty.ann,
        arrowprops=sty.arrowprops,
    )

# Plot
fig, axs = plt.subplots(1, 2, figsize=(10, 5))
for i, h in enumerate(hs[:2]):
    ax: mplaxes.Axes = axs.flat[i]
    ret = h.plot(ax=ax, **sty.base2d)
    ax.set_xlabel("")
    ax.set_ylabel("")
    # Number of ticks in cbar
    ret[1].ax.locator_params(nbins=6) #type: ignore

parts = ["p", "d", "t", r"$^{3}He$", r"$\mathbf{\alpha}$"]
## Side settings
ax = axs[0]
ax.set_xlim(0, 15)
ax.set_ylim(0, 800)
pos = [(6.5, 170), (8.4, 242)]
ds = [3.5, 3.5]
ass = [0, 0]
for i, part in enumerate(parts[:2]):
    annotate(part, pos[i], ds[i], ass[i])

## Transfer settings
ax = axs[1]
pos = [(7.4, 190), (9.7, 250), (11.1, 315), (24, 590), (30.5, 615)]
d = 25
a = -75
ds = [d, d, d, 200, 200]
ass = [a, a, a, -90, -90]
for i, part in enumerate(parts):
    annotate(part, pos[i], ds[i], ass[i])

# Common settings
for ax in axs:
    ax.locator_params(nbins=4)

# Final annotations
phys.utils.annotate_subplots(axs)
for i, ann in enumerate(["Side\n(In)Elastic", "Front\nTransfer"]):
    axs[i].annotate(ann, xy=(0.825, 0.875), xycoords="axes fraction", **sty.ann)

fig.supxlabel(r"$E_{sil}$ [MeV]", x=0.525, y=0.05, ha="center", va="center", fontsize=plt.rcParams["axes.labelsize"])
fig.supylabel(r"$\Delta E_{gas}$ [arb. unit]", fontsize=plt.rcParams["axes.labelsize"])


fig.tight_layout()
fig.savefig(sty.thesis + "e796_pid_gas_sil.pdf", dpi=300)

## Two silicons
plt.close("all")
fig, ax = plt.subplots(figsize=(6, 5))
ax: mplaxes.Axes
ret = hs[-1].plot(ax=ax, norm=mplcolor.LogNorm(), **sty.base2d_nocmin)
cbar = ret[1] #type: ignore
phys.utils.apply_ROOT_colorbar(cbar)

# Annotations
pos = [(9.2, 4), (11.7, 5.45), (13.34, 6.30), (27.8, 13.80), (33.61, 15)]
d = 5
a = -10
ds = [d, d, d, 5, 5]
ass = [a, a, a, -90, -90]
for i, part in enumerate(parts):
    annotate(part, pos[i], ds[i], ass[i])

fig.tight_layout()
fig.savefig(sty.thesis + "e796_pid_two.pdf", dpi=300)

plt.show()
