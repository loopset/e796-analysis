import enum

import pyphysics as phys
import matplotlib.pyplot as plt
import uncertainties as un
import pickle

import sys

sys.path.append("../")
sys.path.append("../dt/")
import styling as sty
import dt

plt.rcParams["axes.labelsize"] = 16

with open("../dt/Inputs/strength_centroids.pkl", "rb") as f:
    stes, _ = pickle.load(f)

# Divided by max orbital occupancy
qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
qd52 = phys.QuantumNumbers(0, 2, 2.5)
qs12 = phys.QuantumNumbers(1, 0, 0.5)
qs = [qd52, qs12, qp12, qp32]

maxoccupancies = {qd52: 4, qs12: 2, qp12: 2, qp32: 4}
divided = {q: v / maxoccupancies[q] * 100 for q, v in stes[0].items()}

fig, ax = plt.subplots(1, 1, figsize=(4, 3), constrained_layout=True)
ax.tick_params(axis="x", which="minor", bottom=False, top=False)
ax.set_xticks(list(range(len(qs))), labels=[q.format() for q in qs])
ax.set_ylabel(r"$\sum C^2S / (2j+1)$ [%]")
ax.set_xlim(-0.5, 3.5)
ax.set_ylim(0, 100)

for i, q in enumerate(qs):
    y = divided[q]
    ax.bar(
        i,
        height=un.nominal_value(y),  # type: ignore
        yerr=un.std_dev(y) if un.std_dev(y) > 0 else None,
        width=0.25,
        align="center",
        label=q.format(),
        **sty.barplot[q],
    )
    fig.savefig(f"./Outputs/strength_{i}.png", dpi=300)


##################### Attempt to stack strengths
plt.close("all")
fig, ax = plt.subplots(1, 1, figsize=(4, 2), constrained_layout=True)
ax.tick_params(axis="x", which="minor", bottom=False, top=False)
ax.set_xticks(list(range(len(qs))), labels=[q.format() for q in qs])
ax.set_ylabel(r"$\sum C^2S^{\pm} / (2j+1)$ [%]")
ax.set_xlim(-0.5, 3.5)
ax.set_ylim(0, 100)

# Add the other datasets
adding = {qd52: un.ufloat(0.34, 0.03), qs12: un.ufloat(0.77, 0.09), qp12: 0, qp32: 0}
adding = {q: v * q.degeneracy() for q, v in adding.items()}  # type: ignore

combined = {}
for q, rem in stes[0].items():
    comb = rem + adding.get(q, 0)
    combined[q] = comb
    print(f"Combined for {q.format_simple()} : {comb:.2uS}")

bottoms = {}
for idx, data in enumerate([stes[0], adding]):
    for i, q in enumerate(qs):
        if not q in bottoms:
            bottoms[q] = 0
        y = data.get(q)
        if y is None:
            continue
        # Get also combined for errobar
        comb = combined[q]
        # Divide by degeneracy and  multiply by 100
        y /= q.degeneracy()
        y *= 100
        comb /= q.degeneracy()
        comb *= 100
        # Get color
        color = sty.barplot[q]["ec"]
        ax.bar(
            i,
            height=un.nominal_value(y),  # type: ignore
            yerr=None if idx == 0 else un.std_dev(comb),
            width=0.25,
            align="center",
            label=q.format(),
            bottom=un.nominal_value(bottoms[q]),
            fc=color if idx == 0 else "none",
            ec=color,
            alpha=0.5 if idx == 0 else 1,
        )
        bottoms[q] = y
fig.savefig("./Outputs/strength_both.png", dpi=300)

plt.show()
