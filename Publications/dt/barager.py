import pyphysics as phys
import matplotlib.pyplot as plt
import uncertainties as unc
import uncertainties.unumpy as unp
import ROOT as r

# Experimental dt data
fit = phys.FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
sfs = phys.SFInterface("../../Fits/dt/Outputs/sfs.root")

# Declare quantumm numbers
q52 = phys.QuantumNumbers(0, 2, 2.5)
q12 = phys.QuantumNumbers(0, 1, 0.5)

# Map to quantum numbers
rem = phys.ShellModel()
assignments = {q52: ["g0"], q12: ["g2", "v0"]}
for q, states in assignments.items():
    rem.data[q] = [
        phys.ShellModelData(
            fit.get(state)[0], best.fSF if (best := sfs.get_best(state)) else 0
        )
        for state in states
    ]
# Adding reactions: B. Fernández-Domínguez PRC 84 (2011)
add = phys.ShellModel()
add.data = {
    q52: [phys.ShellModelData(0, unc.ufloat(0.34, 0.03))],
    q12: [],
}

# Path to theoretical files
path = "../../Fits/dt/"
# YSOX
ysox = phys.ShellModel(
    [
        path + "Inputs/SM/log_O20_O19_ysox_tr_j0p_m1p.txt",
        path + "Inputs/SM/log_O20_O19_ysox_tr_j0p_m1n.txt",
    ]
)

# SFO-tls
sfotls = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
    ]
)

# Modified SFO-tls
mod_sfotls = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
    ]
)
# Modify all sms applying the same cuts
for sm in [rem, ysox, sfotls, mod_sfotls]:
    sm.set_max_Ex(10)
    sm.set_min_SF(0.09)

# rem.print()
# mod_sfotls.print()

# Binding energies
snadd = r.ActPhysics.Particle("21O").GetSn()  # type: ignore
snrem = r.ActPhysics.Particle("20O").GetSn()  # type: ignore

# List all sets of data
labels = ["Exp", "YSOX", "SFO-tls", r"Modified \par SFO-tls"]
removals = [rem, ysox, sfotls, mod_sfotls]
scales = [1, 1, 1, 1]
bs = []
gaps = []
q52str = []
q12str = []
for i, zipped in enumerate(zip(removals, scales)):
    removal, scale = zipped
    b = phys.Barager()
    b.set_removal(removal, snrem, scale)
    b.set_adding(add, snadd)
    b.do_for([q52, q12])
    gaps.append(b.get_gap(q52, q12))
    q52str.append(b.Results[q52].DenRem)
    q12str.append(b.Results[q12].DenRem)
    bs.append(b)
    # print(labels[i]," : ", gaps[-1])
    # b.print()a

# Write to file
with open("./Inputs/o19_barager.txt", "w") as f:
    for label, b in zip(labels, bs):
        res = ""
        for q, br in b.Results.items():
            res += f"    {q.format_simple()}    {br.ESPE:.4f}"
        f.write(f"{label}    {res}\n")

# Plotting
fig, axs = plt.subplots(1, 2, figsize=(11, 4))
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
axs[0].set_xlim(-0.5, len(removals) - 0.5)
axs[0].set_ylabel("N = 8 gap [MeV]")
# Annotation
annx = 1.5
axs[0].annotate(
    "",
    xy=(annx, 7.18),
    xytext=(annx, 5.75),
    arrowprops=dict(arrowstyle="<->", color="royalblue"),
)
axs[0].annotate(
    r"$\Delta \sim 1.4$ MeV", xy=(annx + 0.1, 6.47), color="royalblue", fontsize=16
)

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
axs[1].set_xlim(-0.5, len(removals) - 0.5)
axs[1].set_ylabel("Spe. strengths")
axs[1].axhline(y=6, ls="--", lw=2, color=colors[0], marker="none")
axs[1].axhline(y=2, ls="--", lw=2, color=colors[1], marker="none")
plt.legend(
    fontsize=16, loc="best", markerscale=0.75, fancybox=True, shadow=True, frameon=True
)

fig.tight_layout()
plt.savefig("./Outputs/gap.pdf")
plt.savefig("./Outputs/gap.png", dpi=200)
plt.show()
