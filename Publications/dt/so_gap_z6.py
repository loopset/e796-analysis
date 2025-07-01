from cProfile import label
import matplotlib.pyplot as plt
import uncertainties as un
import uncertainties.unumpy as unp
import numpy as np
import pyphysics as phys

# 16O requires considering small (d,p) adding to 0p3/2
qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
rem: phys.SMDataDict = {
    qp12: [phys.ShellModelData(0, 1.5)],
    qp32: [phys.ShellModelData(6.18, 3.1)],
}
add: phys.SMDataDict = {qp32: [phys.ShellModelData(4.554, 0.23)]}
bar = phys.Barager()
bar.set_removal(rem, phys.Particle("16O").get_sn())
bar.set_adding(add, phys.Particle("17O").get_sn())
bar.do_for([qp12, qp32])
bargap = bar.get_gap(qp12, qp32)
print("0p1/2 ESPE 16O: ", bar.get_ESPE(qp12))
print("0p3/2 ESPE 16O: ", bar.get_ESPE(qp32))
print("SO gap 16O: ", bargap)

# Manual SO gaps
# Define the SO gaps for the other particles
gaps = {r"$^{16}$O": 6.18, r"$^{18}$O": 5.6, r"$^{20}$O": 3.8}

fig, ax = plt.subplots()
x = list(gaps.keys())
y = list(gaps.values())
bary = [bargap] + y[1:]
ax.errorbar(x, unp.nominal_values(y), marker="s", label="No (d,p) vacancies")
ax.errorbar(
    x,
    unp.nominal_values(bary),
    marker="*",
    ms=8,
    ls="--",
    alpha=0.75,
    label="With 16O(d,p) vacancies",
)
ax.set_ylabel(r"$\nu 0\text{p}_{1/2} - \nu 0\text{p}_{3/2}$ [MeV]")
ax.set_xlim(-0.5, len(gaps) - 0.5)
ax.legend()

fig.tight_layout()
fig.savefig("./Outputs/z6_systematics.png")

# Toy figure
fig, ax = plt.subplots()
tx = [0, 1]
ty = unp.nominal_values(y[:2])
# fit
fit = np.poly1d(np.polyfit(tx, ty, 1))
ax.errorbar(tx, ty, marker="s")
ax.errorbar([1, 2], [ty[-1], fit(2)], marker="none", ms=10, ls="--")
ax.set_xlim(-0.5, len(x) - 0.5)
ax.set_ylim(3, 6.5)
ax.set_xticks(list(range(len(x))), labels=x)
ax.annotate(
    "Reanalysis of\n(d,t) literature reactions",
    xy=(0.3, 0.3),
    xycoords="axes fraction",
    ha="center",
    va="center",
    fontsize=12,
)
fig.tight_layout()
fig.savefig("./Outputs/z6_systematics_toy.png")

plt.show()
