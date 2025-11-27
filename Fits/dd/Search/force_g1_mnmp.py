import pyphysics as phys

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as un

from BetaFinder import BetaFinder

o20 = phys.Particle("20O")

# Read interpolation for g1
g1 = BetaFinder("../Outputs/xs/g1_xs.dat", "./g1_Daeh/l2", "202")

# For each beta2 em evaluate the bernstein formula
bem = {"Zanon": 29.5, "Toshio": 17}  ## Beta UP, in units of e2fm4
betasem = {k: phys.BE_to_beta(em, o20, 2) for k, em in bem.items()}

# Evaluate Bernstein formula
betadd = np.arange(0.1, 0.6, 0.025, dtype=float)
mnmps = {}
for k, em in betasem.items():
    mnmps[k] = [phys.simple_bernstein(em, nuclear, o20, 1) for nuclear in betadd]

# Find intersection with E. Khan value for 2+1
ekhan = 3.525
roots = {}
for key, mnmp in mnmps.items():
    spline = phys.create_spline3(betadd, mnmp)
    root = phys.find_root(spline, ekhan, [0, 1])
    roots[key] = root
    print("Key : ", key, " Root : ", root)

# And find scaling factor in our xs
sfs = {}
for k, root in roots.items():
    sfs[k] = 1.0 / (g1.fSpline(root) if g1.fSpline is not None else 1)
    print("Key : ", k, " SF : ", sfs[k])

# States
g2 = BetaFinder("../Outputs/xs/g2_xs.dat", "./g2_Daeh/l2", "202")
g3 = BetaFinder("../Outputs/xs/g3_xs.dat", "./g3_Daeh/l3", "202")
states = {
    "2+1": g1,
    "2+2": g2,
    "3-1": g3,
}

# Betas EM
states_em = {
    "2+1": betasem["Toshio"],
    "2+2": phys.BE_to_beta(5.6, o20, 2, True),  # Toshio value
    "3-1": phys.BE_to_beta(1.6e3, o20, 3, True),  # theoretical paper
}

scaling = sfs["Toshio"]
states_mnmp = {}
for state, finder in states.items():
    finder.scale(scaling)
    states_mnmp[state] = phys.simple_bernstein(states_em[state], finder.fBeta, o20, 1)
    print(f"State {state} Mn/Mp : ", states_mnmp[state])

fig, axs = plt.subplots(2, 2, layout="constrained")
ax: mplaxes.Axes = axs[0, 0]
for k, mnmp in mnmps.items():
    ax.plot(betadd, mnmp, label=k)
ax.axhline(ekhan, ls="--", color="crimson", label="E.Khan")
ax.legend()
ax.set_xlabel(r"$\beta_{dd}(2^+_1)$")
ax.set_ylabel(r"$M_n/M_p$")

ax = axs[0, 1]
g1.plot(ax=ax)
ax.set_title(r"$2^+_1$ scaling")

ax = axs[1, 0]
g2.plot(ax, label="2+2 scaled")
g3.plot(ax, label="3-1 scaled")
ax.axhline(1, color="crimson")
ax.set_ylim(0, 1.5)
ax.legend()

ax = axs[1, 1]
ax.errorbar(
    [k for k in states_mnmp.keys()],
    [un.nominal_value(v) for v in states_mnmp.values()],
    yerr=[un.std_dev(v) for v in states_mnmp.values()],
    marker="o",
    ms=4
)

# Value of E. Khan for 3-1 using exp betapp and 
# theoretical paper beta3
ekhan3 = phys.simple_bernstein(0.606, 0.35, o20, 1/3)
print("E.Khan Mn/Mp 3-1 : ", ekhan3)
ax.hlines(y=ekhan, xmin=-0.25, xmax=0.25, color="crimson")
ax.hlines(y=un.nominal_value(ekhan3), xmin=1.75, xmax=2.25, color="crimson")
ax.set_xlabel("State")
ax.set_ylabel(r"$M_n/M_p$")

# fig.tight_layout()

plt.show()
