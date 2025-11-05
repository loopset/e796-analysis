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

df = uproot.open("../pid/Inputs/pid_front.root:PID_Tree").arrays(["ESil0", "fQave"])  # type: ignore

# Histogram models
ebins = (400, 0, 40)
qbins = (800, 0, 2000)

h = (
    hist.Hist.new.Reg(*ebins, label=r"E$_{sil}$ [MeV]")
    .Reg(*qbins, label=r"$\Delta$E$_{gas}$ [arb. unit]")
    .Double()
)
h.fill(df["ESil0"], df["fQave"])

# Plot
fig, ax = plt.subplots(figsize=(6, 5))
h.plot(ax=ax, **sty.base2d)

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
parts = ["p", "d", "t", r"$^{3}He$", r"$\mathbf{\alpha}$"]
## Transfer settings
pos = [(7.4, 200), (9.20, 270), (10.98, 317), (24, 600), (30.5, 621)]
d = 30
a = -75
ds = [d, d, d, 200, 200]
ass = [a, a, a, -90, -90]
for i, part in enumerate(parts):
    annotate(part, pos[i], ds[i], ass[i])

# Other annotations
# text = "Double vetoed\nand " + r"$\Delta E_{gas}$" + " corrected"
# ax.annotate(text, xy=(0.75, 0.875), xycoords="axes fraction", **sty.ann, fontstyle="italic")

# Common settings
ax.locator_params(nbins=4)

fig.tight_layout()
fig.savefig(sty.thesis + "pid_final.pdf", dpi=300)
plt.show()

