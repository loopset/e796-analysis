import pyphysics as phys
from pyphysics.root_interface import parse_th1, parse_tf1, parse_tgraph
import hist
import uproot
from cycler import cycler

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick

import ROOT as r

import sys

sys.path.append("../")
import styling as sty

file = r.TFile("../../Calibrations/Silicons/Outputs/histos_cal_f0.root")  # type: ignore
hr = file.Get("hFinal")
fr = file.Get("sat0239Pu")

# Read resolutions
layers = ["l0", "f0", "f1"]
res = []
for layer in layers:
    aux = r.TFile(f"../../Calibrations/Silicons/Outputs/histos_cal_{layer}.root") #type: ignore
    graph = parse_tgraph(aux["res"])
    graph[:, 1:2] *= 2.35
    res.append(graph)

# Parse histogram
h = parse_th1(hr)
if h is None:
    raise ValueError("h is None")
# And functions there
hfs = []
for func in hr.GetListOfFunctions():
    hfs.append(parse_tf1(func))
# Parse satellites
sats = []
for key in file.GetListOfKeys():
    if "sat" in key.GetName():
        sats.append(parse_tf1(file.Get(key.GetName())))


# Color style
# plt.rcParams["axes.prop_cycle"] = cycler(color=plt.get_cmap("Dark2").colors)  # type: ignore

# Draw
fig, axs = plt.subplots(1, 2, figsize=(8, 4))

# 3alpha example
ax: mplaxes.Axes = axs[0]
h.plot(ax=ax, color="black", **sty.base1d)
# Main peaks
for func in hfs:
    ax.plot(func[:, 0], func[:, 1], lw=1.25)
# Satellites
for sat in sats:
    ax.plot(sat[:, 0], sat[:, 1], lw=1.25, ls="--")

# Annotate
labels = [r"$^{239}$Pu", r"$^{241}$Am", r"$^{244}$Cm"]
pos = [4.95, 5.3, 5.65]
for label, p in zip(labels, pos):
    ax.annotate(label, xy=(p, 2500), ha="center", va="center", fontsize=14)

# Axis settings
ax.set_ylabel("Counts / 15 keV")
ax.set_xlim(4.75, 6.0)
ax.set_ylim(0, 3500)

## Resolutions
ax: mplaxes.Axes = axs[1]
for i, graph in enumerate(res):
    # Mask missing pads
    if i == 2:
        idxs = [7, 10]
        graph = np.delete(graph, idxs, axis=0)
    mean = np.mean(graph, axis=0)[1]
    print(f"FWHM for {layers[i]}: {mean:.2f} keV")
    err = ax.errorbar(graph[:, 0], graph[:, 1], yerr=graph[:,2], **sty.errorbar_line, label=layers[i])
    ax.axhline(mean, color=err.lines[0].get_color(), ls="--")
ax.legend()
ax.set_xlabel("Silicon pad")
ax.set_ylabel(r"FWHM$_{^{241}Am}$ [keV]")


phys.utils.annotate_subplots(axs)
fig.tight_layout()
fig.savefig(sty.thesis + "sil_cal.pdf", dpi=300)
plt.show()
