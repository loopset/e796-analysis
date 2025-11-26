import pyphysics as phys
import uproot
import hist
import matplotlib as mpl
import matplotlib.ticker as mpltick
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt

import sys

sys.path.append("../")
import styling as sty

exp = uproot.open(
    "../../PostAnalysis/RootFiles/Pipe3/tree_20O_2H_3H_front_juan_RPx.root:Sel_Tree"
).arrays(  # type: ignore
    ["EVertex", "fThetaLight"]
)  # type: ignore

# Histogram
xmodel = (200, 0, 60)
ymodel = (200, 0, 20)
h = (
    hist.Hist.new.Reg(*xmodel, label=r"$\theta_{lab}$ [$\circ$]")
    .Reg(*ymodel, label=r"$E_{lab}$ [MeV]")
    .Double()
)
h.fill(exp["fThetaLight"], exp["EVertex"])

## States to draw
exs = [0, 3.15, 4.62, 14.94]

# Figure
fig, ax = plt.subplots(1, 1, figsize=(5.5, 4.25))
ax: mplaxes.Axes
h.plot(ax=ax, **sty.base2d)
for ex in exs:
    theo = phys.Kinematics(f"20O(d,t)@700|{ex}").get_line3()
    label = f"{ex:.1f}" if ex > 0 else "g.s"
    ax.plot(theo[0], theo[1], label=label)
# Legend
ax.legend(title=r"$E_{x} / MeV$", title_fontsize=12)
## Annotations
ax.annotate(
    rf"$^{{20}}$O(d,t)",
    xy=(0.15, 0.9),
    xycoords="axes fraction",
    fontweight="bold",
    **sty.ann,
)

fig.tight_layout()
fig.savefig(sty.thesis + "20O_dt_kin.pdf", dpi=300)
plt.show()
