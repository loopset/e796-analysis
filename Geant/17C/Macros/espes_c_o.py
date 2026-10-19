import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle

# Define orbitals
qd32 = phys.Orbital.from_str("0d3/2")
qd52 = phys.Orbital.from_str("0d5/2")
qs12 = phys.Orbital.from_str("1s1/2")
# Styles
qs = [qd52, qs12, qd32]
colors = {qd52: "dodgerblue", qs12: "crimson", qd32: "darkorange"}
ls = {qd52: "-", qs12: ":", qd32: "--"}

# Oxygen
oxygen = {
    "A": [8, 9, 10, 11, 12, 13, 14, 15, 16],
    # "p32": [-23.046, -23.443, -23.839, -24.235, -24.631, -25.027, -25.423, -25.707, -25.992],
    # "p12": [-15.541, -16.443, -17.345, -18.247, -19.149, -20.051, -20.953, -20.562, -20.172],
    qs12: [-4.137, -3.993, -3.849, -3.706, -3.562, -3.418, -3.275, -4.249, -5.222],
    qd52: [-5.382, -5.886, -6.390, -6.893, -7.397, -7.901, -8.405, -8.261, -8.117],
    qd32: [1.038, 0.596, 0.154, -0.288, -0.731, -1.173, -1.615, -1.738, -1.861],
}

# Carbon
carbon = {
    "A": [12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0],
    qs12: [
        -2.416,
        -2.025,
        -1.635,
        -1.491,
        -1.348,
        -1.204,
        -1.060,
        -0.917,
        -0.773,
        -1.747,
        -2.722,
        -2.845,
        -2.968,
    ],
    qd52: [
        1.472,
        0.570,
        -0.332,
        -0.836,
        -1.339,
        -1.843,
        -2.347,
        -2.851,
        -3.354,
        -3.211,
        -3.067,
        -3.509,
        -3.952,
    ],
    qd32: [
        2.671,
        2.974,
        3.277,
        2.834,
        2.392,
        1.950,
        1.508,
        1.065,
        0.623,
        0.500,
        0.377,
        0.324,
        0.271,
    ],
}
# Subtract Z from carbon dict
carbon["A"] = [a - 6 for a in carbon["A"]]


# Plot
plt.rcParams["axes.labelsize"] = 14
plt.rcParams["xtick.labelsize"] = 12
plt.rcParams["ytick.labelsize"] = 12

fig, axs = plt.subplots(
    1, 2, figsize=(4.5, 2.5), sharey=True, sharex=True, constrained_layout=True
)
dicts = [carbon, oxygen]
magic = [[("16", (16.7, -1))], [("14", (13.25, -5.75)), ("16", (16, -3.2))]]
labels = ["C", "O"]
for i, ax in enumerate(axs):
    data = dicts[i]
    for q in qs:
        ax.plot(
            data["A"],
            data[q],
            color=colors[q],
            ls=ls[q],
            label=q.format_simple(),
        )
    # Zero
    ax.axhline(0, color="black", ls="--", lw=0.5)
    # Magic numbers
    for m, pos in magic[i]:
        ax.annotate(
            m,
            xy=pos,
            ha="center",
            va="center",
            fontsize=12,
            bbox=dict(boxstyle=f"circle,pad=0.2", fc="none", ec="black", lw=0.75),
        )
    # Labels
    ax.annotate(
        labels[i],
        xy=(0.9, 0.9),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=14,
        fontweight="bold",
    )
    # Orbital names
    if i == 1:
        for q in qs:
            ax.annotate(
                q.format(),
                xy=(6.5, data[q][0]),
                ha="center",
                va="center",
                fontsize=12,
                color=colors[q],
            )
    # Axis limits
    ax.set_ylim(-10, 5)
    ax.set_xlim(4)

# Titles
fig.supxlabel("Neutron number", x=0.55, fontsize=plt.rcParams["axes.labelsize"])
fig.supylabel(r"$\nu$ ESPE [MeV]", y=0.55, fontsize=plt.rcParams["axes.labelsize"])

# Save
fig.savefig("./Outputs/espes_c_o_sfo-tls.pdf", dpi=300)
fig.savefig("./Outputs/espes_c_o_sfo-tls.png", dpi=300)

plt.show()
