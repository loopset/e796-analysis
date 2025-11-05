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
    return 564.711 - 0.283174 * x


# Histogram models
ebins = (400, 0, 40)
qbins = (800, 0, 2000)

# Before histo
dfbef = uproot.open("./Inputs/pid_nocorr_veto.root:PID_Tree")
hbef = (
    hist.Hist.new.Reg(*ebins, label=r"$E_{sil}$ [MeV]")
    .Reg(*qbins, label=r"$\Delta E_{gas}$ [arb. unit]")
    .Double()
)
hbef.fill(dfbef["ESil0"], dfbef["fQave"])  # type: ignore

# Read uncorrected PID cut
cut = uproot.open("../../Macros/PID/Cuts/unpid_tritons_f0.root")["CUTG"].values()

# # After histo
# dfafter = uproot.open("../pid/Inputs/pid_front.root:PID_Tree").arrays(["ESil0", "fQave"])  # type: ignore
# # Correct for difference... PID correction should be taken from the center of the exp distribution
# # not the offset
# offset = -47
# dfafter["fQave"] = dfafter["fQave"] + offset  # type: ignore
# hafter = (
#     hist.Hist.new.Reg(*ebins, label=r"$E_{sil}$ [MeV]")
#     .Reg(*qbins, label=r"$\Delta E_{gas}$ [arb. unit]")
#     .Double()
# )
# hafter.fill(dfafter["ESil0"], dfafter["fQave"])

# Draw
fig, axs = plt.subplots(1, 2, figsize=(9, 4.5))

# Before
ax: mplaxes.Axes = axs[0]
hbef.plot(ax=ax, **sty.base2d, cbar=False)
ax.plot(cut[0], cut[1], color="green")
ax.axvspan(3.5, 4, ymin=0.225, ymax=0.32, color="crimson", alpha=0.5)
ax.annotate("With double veto", xy=(0.75, 0.9), xycoords="axes fraction", **sty.ann, fontstyle="italic")

# Correction
ax = axs[1]
ax.errorbar(xprof, yprof, yerr=yerr, ls="none", marker="s", ms=4)
ax.set_ylim(400, 580)
# Draw fit
x = np.linspace(70, 310, 2)
y = pid_corr(x)
ax.plot(x, y, zorder=3, color="crimson", label="Linear fit")
ax.legend()
ax.set_xlabel("SP.Z [mm]")
ax.set_ylabel(r"$\Delta E_{gas}$ [arb. unit]")
# axin.annotate(
#     r"$E_{sil} \in [3.5, 4]\;MeV$",
#     xy=(0.375, 0.35),
#     xycoords="axes fraction",
#     ha="center",
#     va="center",
#     fontsize=10,
# )

phys.utils.annotate_subplots(axs)

fig.tight_layout()
fig.savefig(sty.thesis + "pid_corr.pdf", dpi=300)
plt.show()
