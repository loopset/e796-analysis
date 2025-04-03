from cycler import cycler
import matplotlib as mpl
import matplotlib.axes as mplaxes
import matplotlib.patches as mplpat
import matplotlib.pyplot as plt
import ROOT as r
import uncertainties as un
import uncertainties.unumpy as unp
import pyphysics as phys
import numpy as np

# Read the data
with r.TFile("../../Fits/pp/Search/Outputs/betas.root") as f:  # type: ignore
    keys = f.Get("Names")
    betas = f.Get("Betas")
    ubetas = f.Get("UBetas")

# Declare a particle
p = phys.Particle(8, 20)

# Experimental data
exp = {}
for i, key in enumerate(keys):
    if "Khan" in key:
        continue
    label = "BG"
    if label not in exp:
        exp[label] = []
    exp[label].append(un.ufloat(betas[i], ubetas[i]))

# Labels for axis
labels = [r"$2_{1}^{+}$", r"$3_{1}^{-}$"]

# EM betas
em = [un.ufloat(5.9, 0.2), un.ufloat(1.6e3, 0.0)]
lem = [2, 3]
isUp = [False, True]
em = [phys.BE_to_beta(be, p, l, isUp[i]) for i, (be, l) in enumerate(zip(em, lem))]
# From E. Khan using simple formula
exp_khan = [un.ufloat(0.55, 0.06), un.ufloat(0.35, 0.05)]
khan = [phys.simple_bernstein(em, nuclear, p, 1.0/3) for em, nuclear in zip(em, exp_khan)]

# Calculate ratios for all data
ratios_simple = {}
ratios_gen = {}
for pot, vals in exp.items():
    for d in [ratios_simple, ratios_gen]:
        if pot not in d:
            d[pot] = []
    for i, beta in enumerate(vals):
        simple = phys.simple_bernstein(em[i], beta, p, 1.0 / 3)
        gen = phys.Bernstein(p, beta, em[i], 1.0 /3)
        ratios_simple[pot].append(simple)
        ratios_gen[pot].append(gen.fMnMp)

# Print
print(f"BG Simple 2+1 Mn/Mp {ratios_simple['BG'][0]:.2uS}")
print(f"E. Khan 2+1 {khan[0]:%.2uS}")

# Plot
fig, axs = plt.subplots(1, 2, figsize=(8, 4))
for pot, vals in exp.items():
    axs[0].errorbar(labels, unp.nominal_values(vals), yerr=unp.std_devs(vals), ls="None", marker="s", label=f"{pot} OMP")
axs[0].errorbar(labels, unp.nominal_values(em), yerr=unp.std_devs(em), ls="None", marker="s", label="EM I. Zanon")

colors = plt.cm.tab10.colors[:2] #type: ignore
axs[1].set_prop_cycle(cycler(color=colors))
ratios = {"Simple": ratios_simple, "Gen": ratios_gen}
for formula, d in ratios.items():
    if formula == "Gen":
        marker="o"
    else:
        marker = "s"
    for pot, vals in d.items():
        axs[1].errorbar(labels, unp.nominal_values(vals), yerr=unp.std_devs(vals), ls="None", marker=marker, label=f"{pot} + {formula}")
# Khan results 
axs[1].errorbar(labels, unp.nominal_values(khan), yerr=unp.std_devs(khan), ls="None", marker="s", color="red", label="E. Khan")
for i in [0, 1]:
    center = khan[i].n #type: ignore
    sigma = khan[i].s #type: ignore
    patch = mplpat.Rectangle(xy=(i - 0.5, center - sigma), width=1, height=sigma * 2, color="red", alpha=0.2)
    axs[1].add_patch(patch)

# General settings
for ax in axs.flatten():
    ax: mplaxes.Axes
    ax.set_xlim(-0.5, len(labels) - 0.5)
    ax.legend()

axs[0].set_ylabel(r"$\beta$")
axs[1].set_ylabel(r"$M_{n}/M_{p}$")

fig.tight_layout()
# fig.savefig("./Outputs/omp_comparison.png", dpi=200)
plt.show()
