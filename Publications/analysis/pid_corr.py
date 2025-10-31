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

# Profile histogram
h = uproot.open("./Inputs/pid_corr_prof_3H.root")["hProf3H_f0"]  # type: ignore
xprof = h.axis(0).centers()  # type: ignore
yprof = h.values()
yerr = h.errors()  # type: ignore


def pid_corr(x):
    return 566.425 - 0.291538 * x


# Histogram models
ebins = (400, 0, 40)
qbins = (800, 0, 2000)

# Before histo
dfbef = uproot.open("./Inputs/pid_before_corr.root:PID_Tree")
hbef = (
    hist.Hist.new.Reg(*ebins, label=r"$E_{sil}$ [MeV]")
    .Reg(*qbins, label=r"$\Delta E_{gas}$ [arb. unit]")
    .Double()
)
hbef.fill(dfbef["ESil0"], dfbef["fQave"])  # type: ignore

# After histo
dfafter = uproot.open("../pid/Inputs/pid_front.root:PID_Tree").arrays(["ESil0", "fQave"])  # type: ignore
# Correct for difference... PID correction should be taken from the center of the exp distribution
# not the offset
offset = -47
dfafter["fQave"] = dfafter["fQave"] + offset  # type: ignore
hafter = (
    hist.Hist.new.Reg(*ebins, label=r"$E_{sil}$ [MeV]")
    .Reg(*qbins, label=r"$\Delta E_{gas}$ [arb. unit]")
    .Double()
)
hafter.fill(dfafter["ESil0"], dfafter["fQave"])

# Draw
fig, axs = plt.subplots(1, 2, figsize=(10, 5))

# Before
ax: mplaxes.Axes = axs[0]
hbef.plot(ax=ax, **sty.base2d, cbar=False)

# Inset
axin = ax.inset_axes((0.50, 0.50, 0.45, 0.45))
axin.errorbar(xprof, yprof, yerr=yerr, ls="none", marker="s", ms=4)
axin.set_ylim(400, 580)
# Draw fit
x = np.linspace(70, 310, 2)
y = pid_corr(x)
axin.plot(x, y, zorder=3, color="crimson", label="Linear fit")
axin.legend()
axin.tick_params(labelsize=10)
axin.set_xlabel("SP.Z [mm]", fontsize=12)
axin.set_ylabel(r"$\Delta E_{gas}$ [arb. unit]", fontsize=12)
axin.annotate(
    r"$E_{sil} \in [3.5, 4]\;MeV$",
    xy=(0.375, 0.35),
    xycoords="axes fraction",
    ha="center",
    va="center",
    fontsize=10,
)

# After
ax = axs[1]
hafter.plot(ax=ax, **sty.base2d, cbar=False)

labels = ["Before", "After"]
for i, ax in enumerate(axs):
    ax.set_xlabel("")
    ax.set_ylabel("")
    ax.annotate(
        labels[i],
        xy=(0.15, 0.85),
        xycoords="axes fraction",
        **sty.ann,
        fontweight="bold"
    )

# Common axis names
fig.supxlabel(
    r"$E_{sil}$ [MeV]",
    x=0.525,
    y=0.05,
    ha="center",
    va="center",
    fontsize=plt.rcParams["axes.labelsize"],
)
fig.supylabel(r"$\Delta E_{gas}$ [arb. unit]", fontsize=plt.rcParams["axes.labelsize"])

phys.utils.annotate_subplots(axs)

fig.tight_layout()
fig.savefig(sty.thesis + "pid_corr.pdf", dpi=300)
plt.show()
