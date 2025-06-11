import pyphysics as phys
import uproot
import hist
import matplotlib.pyplot as plt
import numpy as np

import sys

sys.path.append("../")
from histos import mPID
from styling import base2d


data = uproot.open("./Inputs/pid_front.root:PID_Tree").arrays()  # type: ignore

# Read cuts
which = ["2H", "3H"]
cuts = []
for w in which:
    name = f"../../PostAnalysis/Cuts/LightPID/pid_{w}.root"
    file = uproot.open(name)
    cuts.append(file["CUTG"].values())
    

hPID = hist.Hist.new.Reg(*mPID[0], label=r"$\Delta$E$_{\text{Sil}}$ [MeV]").Reg(*mPID[1], label=r"$\Delta$E$_{\text{gas}}$ [arb. unit]").Double()
hPID.fill(data.ESil0, data.fQave)

fig, ax = plt.subplots(1, 1, figsize=(5,5))
hPID.plot(**base2d)


def annotate(label: str, pos: tuple, l: float, a: float):
    xt = pos[0] + l * np.cos(np.radians(a))
    yt = pos[1] + l * np.sin(np.radians(a))
    ax.annotate(
        label,
        xy=pos,
        xytext=(xt, yt),
        ha="center",
        va="center",
        arrowprops=dict(arrowstyle="-"),
        fontsize=16,
        weight="bold",
    )


labels = ["p", "d", "t", "3He", r"$\mathbf{\alpha}$"]
poss = [(7.4, 190), (9.7, 250), (11.1, 315), (24, 590), (30.5, 615)]
d = 20
a = -75
ds = [d, d, d, 200, 200]
ass = [a, a, a, -90, -90]
for i, l in enumerate(labels):
    annotate(l, poss[i], ds[i], ass[i])
# # Draw cuts
# for cut in cuts:
#     ax.plot(cut[0], cut[1], lw=1.25)

# Axis limits
# ax.set_xlim(0, 17)
# ax.set_ylim(0, 950)
fig.tight_layout()
fig.savefig("./Outputs/pid_front.pdf")
plt.show()