import pyphysics as phys
import uproot
import hist
import matplotlib.pyplot as plt

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
    

hPID = hist.Hist.new.Reg(*mPID[0], label=r"E [MeV]").Reg(*mPID[1], label=r"$\Delta$E [arb. unit]").Double()
hPID.fill(data.ESil0, data.fQave)

fig, ax = plt.subplots(1, 1, figsize=(6,5))
hPID.plot(**base2d)
# Draw cuts
for cut in cuts:
    ax.plot(cut[0], cut[1], lw=1.25)

# Axis limits
ax.set_xlim(0, 17)
ax.set_ylim(0, 950)
fig.tight_layout()
fig.savefig("./Outputs/pid_front_cuts.pdf")
plt.show()