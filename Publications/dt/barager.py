import sys

sys.path.append("/media/Data/E796v2/Python/")

import matplotlib.pyplot as plt
import uncertainties as unc
import uncertainties.unumpy as unp
import ROOT as r

plt.style.use("../../Python/actroot.mplstyle")

import shell_model as sm
from interfaces import FitInterface
from physics import Barager

path = "/media/Data/E796v2/Fits/dt/"

# Experimental dt data
inter = FitInterface(
    path + "Outputs/interface.root",
    path + "Outputs/fit_juan_RPx.root",
    path + "Outputs/sfs.root",
)
# Add systematic 25% error
inter.add_systematic()

# Declare quantumm numbers
q52 = sm.QuantumNumbers(0, 2, 2.5)
q12 = sm.QuantumNumbers(0, 1, 0.5)

# Removal reactions
rem = inter.map(
    {
        q52: ["g0"],
        q12: ["g2", "v0", "v1", "v4"],
    }
)

# Adding reactions: B. Fernández-Domínguez PRC 84 (2011)
add = sm.ShellModel()
add.data = {q52: [sm.ShellModelData(unc.ufloat(0, 0), unc.ufloat(0.34, 0.03))], q12: []}

# YSOX
ysox = sm.ShellModel(
    (
        [
            path + "Inputs/SM/log_O20_O19_ysox_tr_j0p_m1p.txt",
            path + "Inputs/SM/log_O20_O19_ysox_tr_j0p_m1n.txt",
        ]
    )
)

# SFO-tls
sfotls = sm.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
    ]
)

# Modify all sms applying the same cuts
for sm in [rem, ysox, sfotls]:
    sm.set_max_Ex(10)
    sm.set_min_SF(0.09)

rem.print()

# Binding energies
snadd = r.ActPhysics.Particle("21O").GetSn()
snrem = r.ActPhysics.Particle("20O").GetSn()

# List all sets of data
labels = ["p-norm", "d-norm", "YSOX", "SFO-tls"]
removals = [rem, rem, ysox, sfotls]
scales = [1, 1.42, 1, 1]
res = []
gaps = []
q52str = []
q12str = []
for i, zipped in enumerate(zip(removals, scales)):
    removal, scale = zipped
    b = Barager()
    b.set_removal(removal, snrem, scale)
    b.set_adding(add, snadd)
    b.do_for([q52, q12])
    gaps.append(b.get_gap(q52, q12))
    q52str.append(b.Results[q52].DenRem)
    q12str.append(b.Results[q12].DenRem)
    # print(labels[i]," : ", gaps[-1])
    # b.print()


# Plotting
fig, axs = plt.subplots(1, 2, figsize=(9, 4))
axs[0].errorbar(
    labels,
    unp.nominal_values(gaps),
    yerr=unp.std_devs(gaps),
    fmt="o",
    color="dodgerblue",
    mfc="none",
    markersize=10,
    capsize=5,
)
axs[0].set_xlim(-0.5, 3.5)
axs[0].set_ylabel("N = 8 gap [MeV]", loc="top")
# Annotation
annx = 1.5
axs[0].annotate("", xy=(annx, 7.18), xytext=(annx, 5.75), arrowprops=dict(arrowstyle="<->", color="royalblue"))
axs[0].annotate(r"$\Delta \sim 1.4$ MeV", xy=(annx + 0.1, 6.47), color="royalblue", fontsize=16)

colors = ["hotpink", "orange"]
legends = [r"d$_{5/2}$", r"p$_{1/2}$"]
for i, st in enumerate([q52str, q12str]):
    axs[1].errorbar(
        labels,
        unp.nominal_values(st),
        yerr=unp.std_devs(st),
        fmt="o",
        color=colors[i],
        mfc="none",
        ms=10,
        capsize=5,
        label=legends[i],
    )
axs[1].set_xlim(-0.5, 3.5)
axs[1].set_ylabel("Spe. strengths", loc="top")
axs[1].axhline(y=6, ls="--", lw=2, color=colors[0])
axs[1].axhline(y=2, ls="--", lw=2, color=colors[1])
plt.legend(fontsize=16, loc="best", markerscale=0.75)
plt.tight_layout()
plt.savefig("./Outputs/gap.pdf")
plt.show()
