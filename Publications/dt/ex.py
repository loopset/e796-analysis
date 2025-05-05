import pyphysics as phys
import uproot
import hist
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt
import ROOT as r  # type: ignore

exp = uproot.open(
    "../../PostAnalysis/RootFiles/Pipe3/tree_20O_2H_3H_front_juan_RPx.root:Sel_Tree"
).arrays(["Ex"])  # type: ignore

# Ex histogram
nbins = 100
exmin = -5
exmax = 25
# Ground-state
gscut = 1
gsfactor = 0.5
hExgs = hist.Hist.new.Reg(nbins, exmin, exmax, label=r"E$_{\mathrm{x}}$ [MeV]").Double()
gsbin = hExgs.axes[0].index(gscut)
hExgs.fill(exp[exp.Ex < gscut].Ex)
hExgs *= gsfactor
# Other than gs
hEx = hExgs.copy()
hEx.reset()
hEx.fill(exp[exp.Ex >= gscut].Ex)


# Fit interface
inter = phys.FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
# Modify function for gs
inter.fFuncs["g0"] = (lambda old_f: lambda x: [gsfactor * y for y in old_f(x)])(
    inter.fFuncs["g0"]
)

# Draw
fig, ax = plt.subplots(1, 1, figsize=(11, 5))
ax: mplaxes.Axes
# Ex histograms in two regions
hExgs[:gsbin].plot(
    histtype="errorbar",
    yerr=True,
    lw=1,
    color="black",
    flow=None,
    marker="none",
    xerr=True,
    capsize=0,
    label="Exp."
)  # type: ignore
hEx[gsbin:].plot(
    histtype="errorbar",
    yerr=True,
    lw=1,
    color="black",
    flow=None,
    marker="none",
    xerr=True,
    capsize=0,
)  # type: ignore
ax.set_ylabel(f"Counts / {(exmax - exmin) / nbins * 1000:.0f} keV")

# Indivual fits
color = "dodgerblue"
for state in inter.fEx.keys():
    inter.plot_func(state, nbins, exmin, exmax, lw=1, color=color)

# Global fit in two regions
if inter.fGlobal is not None:
    low = inter.fGlobal[inter.fGlobal[:, 0] < gscut]
    low[:, 1] *= gsfactor
    up = inter.fGlobal[inter.fGlobal[:, 0] >= gscut]
    for i, data in enumerate([low, up]):
        ax.plot(
            data[:, 0],
            data[:, 1],
            color="red",
            lw=1.5,
            label="Global fit" if i == 0 else None,
        )

# Phase spaces
inter.fHistPS["ps0"].plot(
    yerr=False,
    lw=1,
    color="orange",
    flow="none",
    label="1n phase space",
    hatch=r"\\"
)
## 2n PS has null amplitude

# Sn
p = r.ActPhysics.Particle("19O")  # type: ignore
ax.axvline(p.GetSn(), label=r"S$_{\mathrm{n}}$", color="purple", lw=2)
ax.axvline(p.GetS2n(), label=r"S$_{\mathrm{2n}}$", color="pink", lw=2)


# Annotations
ax.annotate(rf"g.s $\times$ {gsfactor:.2f}", xy=(1, 400), fontsize=14)

# Legend
ax.legend(fontsize=14)

# Limits
ax.set_xlim(-3, 27)
fig.tight_layout()
fig.savefig("./Outputs/ex.pdf")
fig.savefig("./Outputs/ex.eps")
plt.show()
