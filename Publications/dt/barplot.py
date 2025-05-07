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
    phys.QuantumNumbers.from_str("0p1/2"): dict(fc="none", ec="green", hatch=r"--"),
    phys.QuantumNumbers.from_str("0p3/2"): dict(fc="none", ec="dimgray", hatch="//"),
    phys.QuantumNumbers.from_str("0d5/2"): dict(fc="none", ec="dodgerblue", hatch=".."),
    phys.QuantumNumbers.from_str("1s1/2"): dict(fc="none", ec="crimson", hatch=r"\\"),
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
    "g0": (unrebin, phys.QuantumNumbers.from_str("0d5/2")),
    "g1": (unrebin, phys.QuantumNumbers.from_str("1s1/2")),
    "g2": (unrebin, phys.QuantumNumbers.from_str("0p1/2")),
    "v0": (unrebin, phys.QuantumNumbers.from_str("0p1/2")),
    "v1": (rebin, phys.QuantumNumbers.from_str("0p3/2")),
    "v2": (rebin, phys.QuantumNumbers.from_str("0p3/2")),
    "v3": (rebin, phys.QuantumNumbers.from_str("0p3/2")),
    "v4": (rebin, phys.QuantumNumbers.from_str("0p3/2")),
    "v5": (rebin, phys.QuantumNumbers.from_str("0p3/2")),
    "v6": (rebin, phys.QuantumNumbers.from_str("0p3/2")),
    "v7": (rebin, phys.QuantumNumbers.from_str("0p3/2")),
}  # skip v6
# Equivalences
equiv = {
    phys.QuantumNumbers.from_str("0p1/2"): "l = 1",
    phys.QuantumNumbers.from_str("0p3/2"): "l = 1",
    phys.QuantumNumbers.from_str("0d5/2"): "l = 2",
    phys.QuantumNumbers.from_str("1s1/2"): "l = 0",
}

useBest = False
whichBest = ["v5", "v7"]
exp = defaultdict(list)
for state, (df, q) in assignments.items():
    ex, sigma = states.get(state)
    sf = next((e for e in df.get(state) if e.fName == equiv[q]), None)
    if useBest and state in whichBest:
        sf = df.get_best(state)
        if sf is not None:
            q = next((k for k, v in equiv.items() if v == sf.fName), None)
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
        # corr.n += 1  ## models start at n = 0
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


# for df in [exp, mod, plain]:
#     normalize_to_gs(df)


## Strengths
def get_strength(q: phys.QuantumNumbers, data: dict) -> float | un.UFloat | None:
    if q in data:
        ste = sum(e.sf for e in data[q])
        return ste
    else:
        return None


# Function to plot
def style_plot(data: dict, ax: mplaxes.Axes, errorbar: bool = False):
    aux = copy.deepcopy(data)
    # Scale gs 
    factor = 0.5
    sfval = 0
    for q, vals in aux.items():
        if q == phys.QuantumNumbers(0, 2, 2.5):
            for val in vals:
                val.sf *= factor
                sfval = val.sf
        ax.bar(
            unp.nominal_values([e.ex for e in vals]),
            unp.nominal_values([e.sf for e in vals]),
            yerr=unp.std_devs([e.sf for e in vals]) if errorbar else None,
            width=0.350,
            align="center",
            label=q.format(),
            **(styles[q] if q in styles else {}),
        )
    ax.annotate(fr"gs $\times$ {factor:.1f}", xy=(0.35, un.nominal_value(sfval) * 0.8), fontsize=14)


# Figure
fig, axs = plt.subplots(3, 1, figsize=(11, 7), sharex=True, sharey=True)
# Experimental
ax: mplaxes.Axes = axs[0]
style_plot(exp, ax=ax)

# Modified SFO-tls
ax = axs[1]
style_plot(mod, ax)
# ax.set_ylabel(r"C$^2$S / C$^2$S$_{\mathrm{gs}}$")
ax.set_ylabel(r"C$^2$S")

# Plain SFO-tls
ax = axs[2]
style_plot(plain, ax=ax)
ax.set_xlabel(r"E$_{x}$ [MeV]")

# Common axis settings
labels = ["Exp", "Modified SFO-tls", "SFO-tls"]
for i, a in enumerate(axs.flatten()):
    a: mplaxes.Axes
    # a.grid(True, which="both", axis="y")
    a.annotate(
        rf"\textbf{{{labels[i]}}}",
        xy=(0.5, 0.875),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=14,
    )
    a.legend(ncol=2, fontsize=12)
fig.suptitle(r"${}^{19}$O C$^2$S", fontsize=18)
fig.tight_layout()
fig.savefig("./Outputs/barplot.png", dpi=200)
fig.savefig("./Outputs/barplot.pdf")

## Additional information
fig, axs = plt.subplots(1, 3, figsize=(11, 5))
# Strengths
ax: mplaxes.Axes = axs[0]
for i, d in enumerate([exp, mod, plain]):
    x = []
    stes = []
    for q in exp.keys():
        ste = get_strength(q, d)
        if ste is not None:
            x.append(q.format())
            stes.append(ste)
    ax.errorbar(
        x,
        unp.nominal_values(stes),
        yerr=unp.std_devs(stes),
        marker="s",
        label=labels[i],
    )
ax.legend()
ax.set_xlabel("nlj")
ax.set_ylabel("Spe. strength")

# States based on others
ax = axs[1]
based = exp
for q, vals in based.items():
    vals[:] = [v for v in vals if unp.nominal_values(v.ex) > 10.5]
based = {k: v for k, v in based.items() if len(v) > 0}
ref = based.get(phys.QuantumNumbers.from_str("0p3/2"))
if ref is None:
    plt.show()
    raise SystemExit
based_ref = ref[0]
based_norm = copy.deepcopy(based)
for q, vals in based_norm.items():
    for v in vals:
        v.sf /= based_ref.sf
style_plot(based, ax=ax)
ax.set_ylabel(r"C$^2$S")

# Same but normalized to first
ax = axs[2]
style_plot(based_norm, ax=ax)
ax.set_ylabel(r"C$^2$S / C$^2$S$_{\mathrm{11 MeV}}$")

for ax in axs[1:]:
    ax.locator_params(axis="x", nbins=10)
    ax.grid(True, which="both", axis="y")
    ax.set_xlabel(r"E$_{x}$ [MeV]")
    ax.legend()

fig.tight_layout()
fig.savefig("./Outputs/extra_bar.png", dpi=200)
fig.savefig("./Outputs/extra_bar.pdf")

plt.show()
