import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt
import uncertainties as un

# Define orbitals
qd32 = phys.Orbital.from_str("0d3/2")
qd52 = phys.Orbital.from_str("0d5/2")
qs12 = phys.Orbital.from_str("1s1/2")
qp12 = phys.Orbital.from_str("0p1/2")

# Styles
qs = [qd52, qs12, qd32]
colors = {qd52: "dodgerblue", qs12: "crimson", qd32: "darkorange"}
ls = {qd52: "-", qs12: ":", qd32: "--"}

# SFO-tls
sfo = {
    "A": [8, 9, 10, 11, 12, 13, 14, 15, 16],
    # "p32": [-23.046, -23.443, -23.839, -24.235, -24.631, -25.027, -25.423, -25.707, -25.992],
    qp12: [
        -15.541,
        -16.443,
        -17.345,
        -18.247,
        -19.149,
        -20.051,
        -20.953,
        -20.562,
        -20.172,
    ],
    qs12: [-4.137, -3.993, -3.849, -3.706, -3.562, -3.418, -3.275, -4.249, -5.222],
    qd52: [-5.382, -5.886, -6.390, -6.893, -7.397, -7.901, -8.405, -8.261, -8.117],
    qd32: [1.038, 0.596, 0.154, -0.288, -0.731, -1.173, -1.615, -1.738, -1.861],
}

sfo_gap = [sfo[qd52][i] - sfo[qp12][i] for i in range(len(sfo["A"]))]
mod1_gap = [g - 2.25 for g in sfo_gap]

# Our 20O experimental value
exp_20o = un.ufloat(8.3, 0.5)

# Value in 16O from gs binding energy differences
o16 = phys.Particle("16O")
o15 = phys.Particle("15O")
o17 = phys.Particle("17O")
exp_16o = 2 * o16.get_be() - o15.get_be() - o17.get_be()
print(f"16O gap from BE: {exp_16o:.3f} MeV")


plt.rcParams["xtick.labelsize"] = 12
plt.rcParams["ytick.labelsize"] = 12
fig, ax = plt.subplots(figsize=(4, 2.75), constrained_layout=True)
ax.plot(sfo["A"], sfo_gap, color="green", label="SFO-tls")
ax.plot(sfo["A"], mod1_gap, color="green", ls="--", label="Mod1")
# Exp values
ax.errorbar(12, exp_20o.n, yerr=exp_20o.s, marker="s", color="dodgerblue", label="Exp")
ax.errorbar(8, exp_16o, yerr=0, marker="s", color="dodgerblue")

# Axis settings
ax.set_ylim(5, 15)
ax.set_xlim(7, 17)
ax.set_xlabel("Neutron number N", fontsize=14)
ax.set_ylabel(r"$\Delta_{N = 8}$ [MeV]", fontsize=14)

ax.legend(frameon=True, fancybox=True, loc="upper right", fontsize=10, framealpha=0.9)

plt.show()
