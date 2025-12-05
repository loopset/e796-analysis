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
gap16 = bar.get_gap(qp12, qp32)
# print("0p1/2 ESPE 16O: ", bar.get_ESPE(qp12))
# print("0p3/2 ESPE 16O: ", bar.get_ESPE(qp32))
print("SO gap 16O: ", gap16)

# Define the SO gaps for the other particles
gaps = {r"$^{18}$O": un.ufloat(3.38, 0.10), r"$^{20}$O": un.ufloat(2.87, 0.64)}
theogaps = {r"$^{18}$O":3.75,  r"$^{20}$O": 1.70}  # with Mod2 SFO-tls

labels = [k for k in gaps.keys()]
idxs = list(range(len(labels)))
exp = [v for v in gaps.values()]
theo = [theogaps[k] if k in theogaps else 0 for k in gaps.keys()]

# In relative to 16O
relexp = [v / gap16 for v in exp]  # type: ignore
reltheo = [v / gap16 for v in theo]  # type: ignore

# Relative figure
fig, ax = plt.subplots(figsize=(5, 4))
ax.errorbar(idxs[:1], unp.nominal_values(relexp[:1]), marker="s", ms=8)
ax.set_ylabel(r"$\Delta_{\text{SO}}\; {}^{\text{A}}\text{O} / ^{16}\text{O}$")
ax.set_xlim(-0.5, len(idxs) - 0.5)
ax.set_ylim(0.1, 1.1)
ax.tick_params(axis="x", labelsize=18)
ax.set_xticks(list(range(len(idxs))), labels=labels)
# ax.annotate(
#     "$^{16}$O(d,t)",
#     xy=(0, 5.5 / un.nominal_value(gap16)),
#     ha="center",
#     va="center",
#     fontsize=14,
# )
# ax.annotate(
#     "K.H.Purser et al.\n NPA 132 (1969)",
#     xy=(0, 5.0 / un.nominal_value(gap16)),
#     ha="center",
#     va="center",
#     fontsize=10,
#     style="italic",
# )

ax.annotate(
    "$^{18}$O(d,t)",
    xy=(0, 0.8),
    ha="center",
    va="center",
    fontsize=14,
    color="C0",
)
ax.annotate(
    "G.Mairle et al.\n NPA 280 (1977)",
    xy=(0, 0.725),
    ha="center",
    va="center",
    fontsize=10,
    style="italic",
    color="C0",
)

text0 = ax.annotate(
    "$^{20}$O(d,t)",
    xy=(1, 0.8),
    ha="center",
    va="center",
    fontsize=14,
)
text1 = ax.annotate(
    "This experiment",
    xy=(1, 0.75),
    ha="center",
    va="center",
    fontsize=10,
    style="italic",
)
# Draw span
span = ax.axhspan(0, 1.5, xmin=0.525, xmax=0.97, color="purple", alpha=0.25)

# Horizontal line
ax.axhline(1, ls="dashed", lw=1, color="black")

fig.tight_layout()
# plt.show()
fig.savefig("/media/Data/Docs/SSW/figures/gap_before.png", dpi=600)

## Add 20 point
print(f"Gap 20O : {exp[-1]:.2uS}")
print(f"20O exp reduction: {relexp[-1]:.2uS}")
error = ax.errorbar(
    idxs[1:],
    unp.nominal_values(relexp[1:]),
    yerr=unp.std_devs(relexp[1:]),
    marker="s",
    ms=8,
)
ax.plot([0, 1], unp.nominal_values(relexp), ls="--", color="orange", lw=1.25)
# And theoretical values
for i, t in enumerate(reltheo):
    ax.plot([i - 0.1, i + 0.1], [t] * 2, color="crimson", lw=1.5, alpha=0.75)
    ax.annotate(
        "SFO-tls",
        xy=(i, t + 0.025),
        ha="center",
        va="center",
        fontsize=12,
        color="crimson",
    )

# Format
span.remove()
# text0.set_position((1, 0.75))
# text1.set_position((1, 0.7))

for text in [text0, text1]:
    text.set_color(error.lines[0].get_color())

fig.savefig("/media/Data/Docs/SSW/figures/gap_after.png", dpi=600)
# fig.savefig("/media/Data/Docs/EuNPC/figures/z6_systematics_toy_ours.png", dpi=600)


plt.show()
