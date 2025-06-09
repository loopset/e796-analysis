from collections import defaultdict
import numpy as np
import pyphysics as phys
from pyphysics.actroot_interface import SFInterface
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt
import uncertainties as un
import uncertainties.unumpy as unp
import copy

import sys

sys.path.append("./")
sys.path.append("../")
import styling as sty
import dt

## 1-> Systematic uncertainty from (p,p) normalization
ppgs = phys.parse_txt("../../Fits/pp/Outputs/xs/g0_xs.dat", 3)
ppcomp = phys.Comparator(ppgs)
files = {
    "KD": "../../Fits/pp/Inputs/g0_KD/fort.201",
    "CH89": "../../Fits/pp/Inputs/g0_CH89/fort.201",
    "BG": "../../Fits/pp/Inputs/g0_BG/fort.201",
}
ppcomp.add_models(files)

## 2-> Systematic uncertainty from (d,t) OMPs
dtgs = phys.parse_txt("../../Fits/dt/Outputs/xs/g0_xs.dat", 3)
comp = phys.Comparator(dtgs)
files = {
    "Daeh+Pang": "../../Fits/dt/Inputs/Sys/Daeh_Pang/fort.202",
    "Daeh+HT1p": "../../Fits/dt/Inputs/Sys/Daeh_HT1p/fort.202",
    "Haixia+Pang": "../../Fits/dt/Inputs/Sys/Haixia_Pang/fort.202",
    "Haixia+HT1p": "../../Fits/dt/Inputs/Sys/Haixia_HT1p/fort.202",
}
comp.add_models(files)

fig, axs = plt.subplots(2, 2, figsize=(14, 9))

# Comparator (p,p)
ax = axs[0, 0]
ppcomp.draw(ax=ax)
ax.set_title("20O(p,p) g.s")
ax.set_ylim(1e2, 1e4)
ax.set_yscale("log")

# Comparator (d,t)
ax = axs[0, 1]
comp.draw(ax=ax)
ax.set_title("20O(d,t) g.s")


# Scalings
def get_fraction(val, ref):
    return val / ref


ax = axs[1, 0]
## (p,p)
ref = ppcomp.get_sf("CH89")
pp_handles = []
for model, res in ppcomp.fSFs.items():
    val = get_fraction(res, ref)
    handle = ax.errorbar("(p,p) gs", val.n, yerr=val.s, marker="o", label=model)
    pp_handles.append(handle)

## (d,d)
ref = comp.get_sf("Daeh+Pang")
dt_handles = []
for model, res in comp.fSFs.items():
    val = get_fraction(res, ref)
    handle = ax.errorbar("(d,t) gs", val.n, yerr=val.s, marker="s", label=model)
    dt_handles.append(handle)

leg1 = ax.legend(handles=pp_handles, loc="upper center", title="(p,p) gs norms.")
ax.add_artist(leg1)
leg2 = ax.legend(handles=dt_handles, loc="lower center", title="(d,t) OMP pairs")

## Axis settings
ax.set_title("Relative SFs")
ax.axhline(1, color="crimson", alpha=0.5, ls="dashed")


## C2S with systematic uncertainties
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


# Experimental data
exp = dt.build_sm()
exp_sys = defaultdict(list)
systematic = 0.2
for key, vals in exp.items():
    for val in vals:
        new_val = copy.deepcopy(val)
        usf = np.sqrt(val.SF.s**2 + (val.SF.n * systematic) ** 2)  # type: ignore
        new_val.SF = un.ufloat(un.nominal_value(val.SF), usf)
        exp_sys[key].append(new_val)
# Scale gs
exp_sys[dt.qd52][0].SF *= 0.5

ax = axs[1, 1]
style_plot(exp, ax, True)
for key, vals in exp_sys.items():
    for val in vals:
        ax.errorbar(
            un.nominal_value(val.Ex) + 0.15,
            un.nominal_value(val.SF),
            yerr=un.std_dev(val.SF),
            color="purple",
        )
ax.set_xlabel(r"E$_{\text{x}}$ [MeV]")
ax.set_ylabel("C$^{2}$S")
ax.annotate(
    "unc = stat",
    xy=(0.5, 0.9),
    xycoords="axes fraction",
    fontsize=14,
    ha="center",
    va="center",
)
ax.annotate(
    "unc = stat + (20%) systematic",
    xy=(0.5, 0.8),
    xycoords="axes fraction",
    color="purple",
    fontsize=14,
    ha="center",
    va="center",
)


fig.tight_layout()
fig.savefig("./Outputs/systematic_unc.pdf")
plt.show()
