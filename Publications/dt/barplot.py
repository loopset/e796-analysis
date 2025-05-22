from collections import defaultdict
from dataclasses import dataclass
from typing import Dict, List, Tuple, Union
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import numpy as np
import copy
import uncertainties as un
import uncertainties.unumpy as unp
import ROOT as r
import sys

sys.path.append("./")
sys.path.append("../")
import dt
import styling as sty

r.PyConfig.DisableRootLogon = True  # type: ignore

## Experimental dataset
exp = dt.build_sm()

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


## Normalize to gs
def normalize_to_gs(data: dict) -> None:
    q = phys.QuantumNumbers(0, 2, 5 / 2)
    gs = copy.deepcopy(data[q][0]) if q in data else None
    if gs is not None:
        for q, vals in data.items():
            for val in vals:
                val.sf /= gs.sf


# for df in [exp, mod, plain]:
#     normalize_to_gs(df)


## Strengths
def get_strengths(
    data: phys.SMDataDict,
) -> Dict[phys.QuantumNumbers, Union[float, un.UFloat]]:
    ret = {}
    for q, vals in data.items():
        ret[q] = sum(val.SF for val in data[q])  # type: ignore
    return ret


def get_centroids(
    data: phys.SMDataDict,
) -> Dict[phys.QuantumNumbers, Union[float, un.UFloat]]:
    zero = 0
    ret = {}
    for q, vals in data.items():
        num = 0
        den = 0
        for val in vals:
            num += (2 * q.j + 1) * val.SF * (val.Ex - zero)  # type: ignore
            den += (2 * q.j + 1) * val.SF  # type: ignore
        ret[q] = num / den
    return ret


## T=5/2 isospin treatment
low, up = dt.split_isospin(exp)
lowSFO, upSFO = dt.split_isospin(mod.data)
tfactor = 5  # transformation between T=3/2 and 5/2
for dicti in [up, upSFO]:
    for q, vals in dicti.items():
        for val in vals:
            val.SF *= tfactor  # type: ignore


# Function to plot
def style_plot(
    data: phys.SMDataDict,
    ax: mplaxes.Axes,
    errorbar: bool = False,
):
    aux = copy.deepcopy(data)
    # Scale gs
    factor = 0.5
    sfval = 0
    gsAnnotation = False
    for q, vals in aux.items():
        if q == phys.QuantumNumbers(0, 2, 2.5):
            gsAnnotation = True
            for val in vals:
                val.SF *= factor  # type: ignore
                sfval = val.SF
        ax.bar(
            unp.nominal_values([e.Ex for e in vals]),
            unp.nominal_values([e.SF for e in vals]),
            yerr=unp.std_devs([e.SF for e in vals]) if errorbar else None,
            width=0.350,
            align="center",
            label=q.format(),
            **sty.barplot.get(q, {}),
        )
    if gsAnnotation:
        ax.annotate(
            rf"gs $\times$ {factor:.1f}",
            xy=(0.35, un.nominal_value(sfval) * 0.8),
            fontsize=14,
        )


def strength_plot(data: dict, ax: mplaxes.Axes, label=None) -> None:
    qs = [
        phys.QuantumNumbers(0, 2, 2.5),
        phys.QuantumNumbers(1, 0, 0.5),
        phys.QuantumNumbers(0, 1, 0.5),
        phys.QuantumNumbers(0, 1, 1.5),
    ]
    x = []
    y = []
    for q in qs:
        x.append(q.format())
        if q in data:
            y.append(data[q])
        else:
            y.append(0)
    ax.errorbar(x, unp.nominal_values(y), yerr=unp.std_devs(y), marker="s", label=label)


# Figure
fig, axs = plt.subplots(3, 1, figsize=(11, 7), sharex=True, sharey=True)
# Experimental
ax: mplaxes.Axes = axs[0]
style_plot(exp, ax=ax)

# Modified SFO-tls
ax = axs[1]
style_plot(mod.data, ax)
# ax.set_ylabel(r"C$^2$S / C$^2$S$_{\mathrm{gs}}$")
ax.set_ylabel(r"C$^2$S")

# Plain SFO-tls
ax = axs[2]
style_plot(plain.data, ax=ax)
ax.set_xlabel(r"E$_{x}$ [MeV]")

# Common axis settings
labels = ["Exp", "Modified SFO-tls", "SFO-tls"]
for i, a in enumerate(axs.flatten()):
    a: mplaxes.Axes
    # a.grid(True, which="both", axis="y")
    a.annotate(
        f"{labels[i]}",
        xy=(0.5, 0.875),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=14,
        weight="bold",
    )
    a.legend(ncol=2, fontsize=12)
fig.suptitle(r"${}^{19}$O C$^2$S", fontsize=18)
fig.tight_layout()
fig.savefig("./Outputs/barplot.png", dpi=200)
fig.savefig("./Outputs/barplot.pdf")

## Isospin info
fig, axs = plt.subplots(2, 2, figsize=(11, 9))
ax: mplaxes.Axes = axs[0, 0]
ax.set_title("T = 3/2")
style_plot(low, ax, False)
ax = axs[0, 1]
ax.set_title("T = 5/2")
style_plot(up, ax)
for ax in [axs[0, 0], axs[0, 1]]:
    ax.set_xlabel(r"E$_{\text{x}}$ [MeV]")
    ax.set_ylabel(r"C$^{2}$S")
## Strengths
# Experimental
for i, data in enumerate([low, up]):
    ax = axs[1, i]
    ste = get_strengths(data)
    strength_plot(ste, ax, "Exp")
# Mod SFO-tls
for i, data in enumerate([lowSFO, upSFO]):
    ax = axs[1, i]
    ste = get_strengths(data)
    strength_plot(ste, ax, "Mod SFO-tls")
for ax in [axs[1, 0], axs[1, 1]]:
    ax.set_ylabel("Spe. strength")
    ax.legend()

fig.tight_layout()

fig, axs = plt.subplots(1, 2, figsize=(11, 7))
ax: mplaxes.Axes = axs[0]
for i, data in enumerate([low, lowSFO]):
    spe = get_strengths(data)
    centroids = get_centroids(data)
    x = []
    y = []
    for q in spe:
        x.append(centroids[q])
        y.append(spe[q])
    ax.errorbar(
        unp.nominal_values(x), unp.nominal_values(y), yerr=unp.std_devs(y), marker="o"
    )

fig.tight_layout()
plt.show()

# ## Additional information
# fig, axs = plt.subplots(1, 3, figsize=(11, 5))
# # Strengths
# ax: mplaxes.Axes = axs[0]
# for i, d in enumerate([exp, mod, plain]):
#     if isinstance(d, phys.ShellModel):
#         d = d.data
#     x = []
#     stes = []
#     for q in exp.keys():
#         ste = get_strength(q, d)
#         if ste is not None:
#             x.append(q.format())
#             stes.append(ste)
#     ax.errorbar(
#         x,
#         unp.nominal_values(stes),
#         yerr=unp.std_devs(stes),
#         marker="s",
#         label=labels[i],
#     )
# ax.legend()
# ax.set_xlabel("nlj")
# ax.set_ylabel("Spe. strength")

# # States based on others
# ax = axs[1]
# based = exp
# for q, vals in based.items():
#     vals[:] = [v for v in vals if unp.nominal_values(v.Ex) > 10.5]
# based = {k: v for k, v in based.items() if len(v) > 0}
# ref = based.get(phys.QuantumNumbers.from_str("0p1/2"))
# if ref is None:
#     plt.show()
#     raise SystemExit
# based_ref = ref[0]
# based_norm = copy.deepcopy(based)
# for q, vals in based_norm.items():
#     for v in vals:
#         v.SF /= based_ref.SF  # type: ignore
# style_plot(based, ax=ax)
# ax.set_ylabel(r"C$^2$S")

# # Same but normalized to first
# ax = axs[2]
# style_plot(based_norm, ax=ax)
# ax.set_ylabel(r"C$^2$S / C$^2$S$_{\mathrm{11 MeV}}$")

# for ax in axs[1:]:
#     ax.locator_params(axis="x", nbins=10)
#     ax.grid(True, which="both", axis="y")
#     ax.set_xlabel(r"E$_{x}$ [MeV]")
#     ax.legend()

# fig.tight_layout()
# fig.savefig("./Outputs/extra_bar.png", dpi=200)
# fig.savefig("./Outputs/extra_bar.pdf")

plt.show()
