import pyphysics as phys
from pyphysics.root_interface import parse_th1, parse_tf1
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
fig, ax = plt.subplots()
h.plot(color="black", **sty.base1d)
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
ax.set_xlim(4.5, 6.3)
ax.set_ylim(0, 3500)

fig.tight_layout()
fig.savefig(sty.thesis + "sil_cal.pdf", dpi=300)
plt.show()
