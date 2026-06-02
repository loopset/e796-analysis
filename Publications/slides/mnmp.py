import pyphysics as phys
import matplotlib.pyplot as plt
import uncertainties as un
import uncertainties.unumpy as unp
import sys

sys.path.append("../")
import styling as sty

labels = [r"$2^+_1$", r"$2^+_2$", r"$3^-_1$"]
ratios = [un.ufloat(3.82, 0.41), un.ufloat(5.13, 0.77), un.ufloat(0.55, 0.18)]

fig, ax = plt.subplots(1, 1, figsize=(4, 3), constrained_layout=True)
ax.errorbar(
    labels,
    unp.nominal_values(ratios),
    yerr=unp.std_devs(ratios),
    **sty.errorbar,
    color="black",
    mfc="none",
    mec="black"
)

ax.tick_params(axis="x", which="minor", bottom=False, top=False)
ax.set_ylabel(r"$M_n/M_p$")
ax.set_ylim(0, 6)
ax.set_xlim(-0.5, 2.5)

# Mn/Mp = 1
ax.axhline(1, ls="--", color="black")

# Spans
ax.axhspan(0, 1, color="crimson", alpha=0.1)
ax.axhspan(1, 6, color="dodgerblue", alpha=0.1)

# Savefig
fig.savefig("./Outputs/mnmp.png", dpi=300)

plt.show()
