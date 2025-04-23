from collections import defaultdict
from dataclasses import dataclass
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import numpy as np
import copy
import uncertainties as un
import uncertainties.unumpy as unp
import ROOT as r

r.PyConfig.DisableRootLogon = True  # type: ignore

# Styles
styles = {
    phys.QuantumNumbers.from_str("1p1/2"): dict(fc="none", ec="purple", hatch=r"\\"),
    phys.QuantumNumbers.from_str("1p3/2"): dict(fc="none", ec="crimson", hatch="//"),
    phys.QuantumNumbers.from_str("1d5/2"): dict(fc="none", ec="dodgerblue", hatch=".."),
    phys.QuantumNumbers.from_str("2s1/2"): dict(fc="none", ec="darkgreen", hatch="--"),
}


@dataclass
class Pair:
    ex: float | un.UFloat
    sf: float | un.UFloat


## Experimental
# Ex
states = phys.FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
# Read SFs
unrebin = phys.SFInterface("../../Fits/dt/Outputs/sfs.root")
rebin = phys.SFInterface("../../Fits/dt/Outputs/rebin_sfs.root")
assignments = {
    "g0": (unrebin, phys.QuantumNumbers.from_str("1d5/2")),
    "g1": (unrebin, phys.QuantumNumbers.from_str("2s1/2")),
    "g2": (unrebin, phys.QuantumNumbers.from_str("1p1/2")),
    "v0": (unrebin, phys.QuantumNumbers.from_str("1p1/2")),
    "v1": (rebin, phys.QuantumNumbers.from_str("1p3/2")),
    "v2": (rebin, phys.QuantumNumbers.from_str("1p3/2")),
    "v3": (rebin, phys.QuantumNumbers.from_str("1p3/2")),
    "v4": (rebin, phys.QuantumNumbers.from_str("1p3/2")),
    "v5": (rebin, phys.QuantumNumbers.from_str("1p3/2")),
    "v7": (rebin, phys.QuantumNumbers.from_str("2s1/2")),
}  # skip v6
# Equivalences
equiv = {
    phys.QuantumNumbers.from_str("1p1/2"): "l = 1",
    phys.QuantumNumbers.from_str("1p3/2"): "l = 1",
    phys.QuantumNumbers.from_str("1d5/2"): "l = 2",
    phys.QuantumNumbers.from_str("2s1/2"): "l = 0",
}
exp = defaultdict(list)
for state, (df, q) in assignments.items():
    ex, sigma = states.get(state)
    sf = next((e for e in df.get(state) if e.fName == equiv[q]), None)
    if sf is not None:
        exp[q].append(Pair(ex, sf.fSF))

## Theoretical datasets
# Modified SFO-tls
mod = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)
# Plain SFO-tls
plain = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
    ]
)
for m in [mod, plain]:
    m.set_max_Ex(15)
    m.set_min_SF(0.1)


def transform(sm: phys.ShellModel) -> dict:
    ret = defaultdict(list)
    for q, vals in sm.data.items():
        corr = q
        corr.n += 1  ## models start at n = 0
        ret[q] = [Pair(val.Ex, val.SF) for val in vals]
    return ret


mod = transform(mod)
plain = transform(plain)


## Normalize to gs
def normalize_to_gs(data: dict) -> None:
    q = phys.QuantumNumbers(1, 2, 5 / 2)
    gs = copy.deepcopy(data[q][0]) if q in data else None
    if gs is not None:
        for q, vals in data.items():
            for val in vals:
                val.sf /= gs.sf


for df in [exp, mod, plain]:
    normalize_to_gs(df)


# Function to plot
def style_plot(data: dict, ax: mplaxes.Axes, errorbar: bool = False):
    for q, vals in data.items():
        ax.bar(
            unp.nominal_values([e.ex for e in vals]),
            unp.nominal_values([e.sf for e in vals]),
            yerr=unp.std_devs([e.sf for e in vals]) if errorbar else None,
            width=0.5,
            align="center",
            label=q.format(),
            **(styles[q] if q in styles else {}),
        )


# Figure
fig, axs = plt.subplots(3, 1, figsize=(11, 7), sharex=True, sharey=True)
# Experimental
ax: mplaxes.Axes = axs[0]
style_plot(exp, ax=ax)

# Modified SFO-tls
ax = axs[1]
style_plot(mod, ax)
ax.set_ylabel(r"C$^2$S / C$^2$S$_{\mathrm{gs}}$")

# Plain SFO-tls
ax = axs[2]
style_plot(plain, ax=ax)
ax.set_xlabel(r"E$_{x}$ [MeV]")

# Common axis settings
labels = ["Exp", "Modified SFO-tls", "Plain SFO-tls"]
for i, a in enumerate(axs.flatten()):
    a: mplaxes.Axes
    a.grid(True, which="both", axis="y")
    a.annotate(
        rf"\textbf{{{labels[i]}}}",
        xy=(0.5, 0.875),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=14,
    )
    a.legend(ncol=2, fontsize=12)
fig.suptitle(r"Normalised C$^2$S for ${}^{19}$O", fontsize=18)
fig.tight_layout()
fig.savefig("./Outputs/barplot.png", dpi=200)
fig.savefig("./Outputs/barplot.pdf")
plt.show()
