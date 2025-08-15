import pyphysics as phys
import matplotlib.pyplot as plt
import uncertainties as un
import uncertainties.unumpy as unp
import numpy as np

# 16O requires considering small (d,p) adding to 0p3/2
qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
rem: phys.SMDataDict = {
    qp12: [phys.ShellModelData(0, 1.5)],
    qp32: [phys.ShellModelData(6.18, 3.1)],
}
add: phys.SMDataDict = {qp32: [phys.ShellModelData(4.554, 0.0)]}
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
theogaps = {r"$^{20}$O": 5.64}
fig, ax = plt.subplots(figsize=(5, 4))
x = list(gaps.keys())
y = list(gaps.values())
bary = [bargap] + y[1:]
ax.errorbar(x, unp.nominal_values(y), marker="s", ms=8, label="Exp. / Reana.")
ax.errorbar(list(theogaps.keys()), unp.nominal_values(list(theogaps.values())), marker="h", ms=8, label="Mod SFO-tls")
# ax.errorbar(
#     x,
#     unp.nominal_values(bary),
#     marker="*",
#     ms=8,
#     ls="--",
#     alpha=0.75,
#     label="With 16O(d,p) vacancies",
# )
ax.set_ylabel(r"$\nu 0\text{p}_{1/2} - \nu 0\text{p}_{3/2}$ [MeV]")
ax.set_xlim(-0.5, len(gaps) - 0.5)
ax.tick_params(axis="x", labelsize=18)
ax.legend()

fig.tight_layout()
fig.savefig("./Outputs/z6_systematics.png", dpi=300)
plt.close(fig)

# Toy figure
fig, ax = plt.subplots(figsize=(5, 4))
tx = [0, 1]
ty = unp.nominal_values(y[:2])
# fit
fit = np.poly1d(np.polyfit(tx, ty, 1))
ax.errorbar(tx, ty, marker="s", ms=8)
# ax.errorbar([1, 2], [ty[-1], fit(2)], marker="none", ms=8, ls="--")
ax.set_xlim(-0.5, len(x) - 0.5)
ax.set_ylim(3, 6.5)
ax.tick_params(axis="x", labelsize=18)
ax.set_xticks(list(range(len(x))), labels=x)
ax.annotate(
    "$^{16}$O(d,t)",
    xy=(0, 5.5),
    ha="center",
    va="center",
    fontsize=14,
)
ax.annotate(
    "K.H.Purser et al.\n NPA 132 (1969)",
    xy=(0, 5.25),
    ha="center",
    va="center",
    fontsize=10,
    style="italic"
)

ax.annotate(
    "$^{18}$O(d,t)",
    xy=(1, 5.),
    ha="center",
    va="center",
    fontsize=14,
)
ax.annotate(
    "G.Mairle et al.\n NPA 280 (1977)",
    xy=(1., 4.75),
    ha="center",
    va="center",
    fontsize=10,
    style="italic"
)

ax.annotate(
    "$^{20}$O(d,t)",
    xy=(2, 4.),
    ha="center",
    va="center",
    fontsize=14,
)
ax.annotate(
    "This experiment",
    xy=(2., 3.75),
    ha="center",
    va="center",
    fontsize=10,
    style="italic"
)
# ax.text(
#     0.5,
#     0.5,
#     "Preliminary",
#     transform=ax.transAxes,
#     fontsize=60,
#     color="gray",
#     alpha=0.5,
#     ha="center",
#     va="center",
#     rotation=30,
# )

# Draw span
ax.axhspan(3, 6.5, xmin=0.70, xmax=0.97, color="purple", alpha=0.25)

ax.set_ylabel(r"$\Delta_{\text{SO}}$ [MeV]")
fig.tight_layout()
fig.savefig("./Outputs/z6_systematics_toy.png", dpi=300)

plt.show()
