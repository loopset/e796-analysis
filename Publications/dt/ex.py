import pyphysics as phys
from pyphysics.actroot_interface import FitInterface
import uproot
import hist
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt
import ROOT as r  # type: ignore
import sys

sys.path.append("../")
from styling import styles
from histos import mExdt

exp = uproot.open(
    "../../PostAnalysis/RootFiles/Pipe3/tree_20O_2H_3H_front_juan_RPx.root:Sel_Tree"
).arrays(  # type: ignore
    ["Ex"]
)

# Ex histogram
nbins = mExdt[0]
exmin = mExdt[1]
exmax = mExdt[2]
# Ground-state
gscut = 1
gsfactor = 0.25
hExgs = hist.Hist.new.Reg(*mExdt, label=r"E$_{\mathrm{x}}$ [MeV]").Double()
gsbin = hExgs.axes[0].index(gscut)
hExgs.fill(exp[exp.Ex < gscut].Ex)
hExgs *= gsfactor
# Other than gs
hEx = hExgs.copy()
hEx.reset()
hEx.fill(exp[exp.Ex >= gscut].Ex)


# Fit interface
inter = FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
# Modify function for gs
inter.fFuncs["g0"] = (lambda old_f: lambda x: [gsfactor * y for y in old_f(x)])(
    inter.fFuncs["g0"]
)

# Draw
fig, ax = plt.subplots(1, 1, figsize=(8, 4))
ax: mplaxes.Axes
# Ex histograms in two regions
hExgs[:gsbin].plot(label="Experiment", **styles["ex"])  # type: ignore
hEx[gsbin:].plot(**styles["ex"])  # type: ignore
ax.set_ylabel(f"Counts / {(exmax - exmin) / nbins * 1000:.0f} keV")

# Global fit in two regions
if inter.fGlobal is not None:
    low = inter.fGlobal[inter.fGlobal[:, 0] < gscut]
    low[:, 1] *= gsfactor
    up = inter.fGlobal[inter.fGlobal[:, 0] >= gscut]
    for i, data in enumerate([low, up]):
        ax.plot(
            data[:, 0],
            data[:, 1],
            label="Global fit" if i == 0 else None,
            **styles["global"],
        )

# Indivual fits
color = "dodgerblue"
for i, state in enumerate(inter.fEx.keys()):
    if state in ["v8", "v9", "v10"]:  # exclude peaks at >16 MeV
        continue
    if i < 7:
        ls = "solid"
    else:
        ls = "dashed"
    label = None
    if i == 0:
        label = r"T = 3/2"
    elif i == 7:
        label = r"T = 5/2"
    inter.plot_func(state, nbins, exmin, exmax, lw=1, color=color, ls=ls, label=label)

# Phase spaces
inter.fHistPS["ps0"].plot(
    label="1n phase space", color="orange", hatch="\\\\", **styles["ps"]
)
## 2n PS has null amplitude
inter.fHistPS["ps1"].plot(
    label="2n phase space", color="green", hatch="//", **styles["ps"]
)
# (p,d) contamination
for i, state in enumerate(["v8", "v9", "v10"]):
    inter.plot_func(
        state,
        nbins,
        exmin,
        exmax,
        label="(p,d) background" if i == 0 else None,
        color="grey",
        hatch="xx",
        **styles["ps"],
    )

# Sn
p = r.ActPhysics.Particle("19O")  # type: ignore
ax.axvline(p.GetSn(), color="purple", **styles["sn"])
ax.annotate(
    rf"S$_{{\mathrm{{n}}}} =$ {p.GetSn():.2f} MeV",
    xy=(p.GetSn() + 0.5, 800 * gsfactor),
    fontsize=12,
)
ax.axvline(p.GetS2n(), color="hotpink", **styles["sn"])
ax.annotate(
    rf"S$_{{\mathrm{{2n}}}} =$ {p.GetS2n():.2f} MeV",
    xy=(p.GetS2n() - 5.5, 650 * gsfactor),
    fontsize=12,
)


# Annotations
ax.annotate(rf"g.s $\times$ {gsfactor:.2f}", xy=(0.3, 900 * gsfactor), fontsize=12)

# Legend
handles, labels = ax.get_legend_handles_labels()
ax.legend(handles=[handles[-1]] + handles[:-1], labels=[labels[-1]] + labels[:-1])

# Limits
ax.set_xlim(-3, 25)
fig.tight_layout()
fig.savefig("./Outputs/ex.pdf")
fig.savefig("./Outputs/ex.eps")
plt.show()
