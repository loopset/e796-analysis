from typing import Dict

from sklearn.model_selection import ValidationCurveDisplay
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes

# Particle
part = phys.Particle("20O")

# Orbitals
orbs = [
    phys.QuantumNumbers.from_str("0s1/2"),
    phys.QuantumNumbers.from_str("0p3/2"),
    phys.QuantumNumbers.from_str("0p1/2"),
    phys.QuantumNumbers.from_str("0d5/2"),
    phys.QuantumNumbers.from_str("1s1/2"),
    # phys.QuantumNumbers.from_str("0d3/2"),
]

shift_low = +0.2
shift_up = -0.1
shifts = [0, shift_low, shift_up, shift_low, shift_up]

# Magic numbers
magic = [2, 8, 20]

# Orbit parameters
width = 0.7
lw = 1.5
ocolor = "dimgrey"

# Nucleon parameters
pcolor = "crimson"
ncolor = "dodgerblue"

# Figure
fig, ax = plt.subplots(figsize=(4, 4))
ax: mplaxes.Axes

for i, nucleons in enumerate([part.Z, part.N]):
    xlow = (i + 1) - width / 2
    xup = (i + 1) + width / 2
    # Orbitals
    # Nucleons
    last_n = 0
    # aux_magic = 0
    for j, orb in enumerate(orbs):
        y = j + shifts[j]
        ax.plot([xlow, xup], [y] * 2, lw=lw, color=ocolor)
        for count, nucleon in enumerate(range(last_n, nucleons)):
            if count < orb.degeneracy():
                bw = width / orb.degeneracy()
                xn = xlow + bw / 2 + count * bw
                mcolor = pcolor if i == 0 else ncolor
                ax.plot(xn, y, color=mcolor, mfc=mcolor, marker="o", markersize=10)
                last_n += 1
        # aux_magic += orb.degeneracy()
        # if aux_magic in magic:
        #     ax.annotate(
        #         f"{aux_magic}",
        #         xy=(i + 1, y + 0.5),
        #         ha="center",
        #         va="center",
        #         fontsize=14,
        #         fontstyle="italic",
        #     )


# Orbital name and magic numbers
cmagic = 0
for i, orb in enumerate(orbs):
    y = i + shifts[i]
    ax.annotate(orb.format(), xy=(1.5, y), ha="center", va="center", fontsize=14)
    cmagic += orb.degeneracy()
    if cmagic in magic:
        ax.annotate(
            f"{cmagic}",
            xy=(1.5, i + 0.5),
            ha="center",
            va="center",
            fontsize=12,
            fontstyle="italic",
        )

# Column name
for i, label in enumerate([r"$\pi$", r"$\nu$"]):
    ax.annotate(label, xy=(i + 1, -0.5), ha="center", va="center", fontsize=20)


# Axis settings
ax.set_ylim(-0.75)
ax.set_axis_off()

fig.tight_layout()
fig.savefig(f"./Outputs/sm_{part.symbol}.pdf")

# Tiny image for xs explanation
plt.close("all")
fig, ax = plt.subplots(figsize=(1.5, 2.5))
ax: mplaxes.Axes

which = "n"
ncolor = "dodgerblue" if which == "n" else "crimson"
occupancies: Dict[phys.QuantumNumbers, int] = {
    orbs[0]: 2,
    orbs[1]: 4,
    orbs[2]: 2,
    orbs[3]: 2,
    orbs[4]: 1,
}
vacancies: Dict[phys.QuantumNumbers, int] = {
    # phys.QuantumNumbers.from_str("0d5/2"): 1,
    phys.QuantumNumbers.from_str("1s1/2"): 1,
}
filename = "19O_1s12.pdf"
print(f"Filename : {filename}")
# X axis settings
width = 1
xlow = 1 - width / 2
xup = 1 + width / 2
for j, orb in enumerate(orbs):
    y = j + shifts[j]
    ax.plot([xlow, xup], [y] * 2, lw=lw, color=ocolor)
    if orb in occupancies:
        bw = width / orb.degeneracy()
        count = 0
        # Occupied
        for n in range(occupancies[orb]):
            xn = xlow + bw / 2 + count * bw
            ax.plot(xn, y, color=ncolor, mfc=ncolor, marker="o", ms=8)
            count += 1
        # Vacancies
        vac = 0
        if orb.l == 1:
            vac = orb.degeneracy() - occupancies[orb]
        if orb in vacancies:
            vac = vacancies[orb]
        for h in range(vac):
            xn = xlow + bw / 2 + count * bw
            ax.plot(xn, y, color=ncolor, marker="o", ms=8)
            count += 1

# Orbital name and magic numbers
for i, orb in enumerate(orbs):
    y = i + shifts[i]
    ax.annotate(
        orb.format(), xy=(1 + width / 2 + 0.3, y), ha="center", va="center", fontsize=14
    )
ax.set_xlim(0.5, 1.8)
ax.set_axis_off()
fig.savefig(f"./Outputs/{filename}")

plt.show()
