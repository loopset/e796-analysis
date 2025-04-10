from matplotlib.legend import Legend
import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes

# Read data
files = {
    r"$2^+_1$": "../Outputs/xs/g1_xs.dat",
    r"$2^+_2$": "../Outputs/xs/g2_xs.dat",
    r"$3^+_1$": "../Outputs/xs/g3_xs.dat",
    r"$0^+_2$": "../Outputs/xs/g7_xs.dat",
}

# Experimental data
exp = {k: phys.parse_txt(file, 3) for k, file in files.items()}

# And compare!
comps = {k: phys.Comparator(e) for k, e in exp.items()}

# Map fortran files to states
files = ["202", "203", "204", "205"]  # dictionaries are ordered in python
dirs = {
    "iblock=0": "./Inputs/fresco_step/iblock0/",
    "iblock=5": "./Inputs/fresco_step/iblock5/",
}

for i, (k, c) in enumerate(comps.items()):
    for ib, dir in dirs.items():
        file = dir + "fort." + files[i]
        c.add_model(ib, file)

# Append for 0+2 the previous calculation
last = next(reversed(comps.values()))
last.add_model("gs", "../Inputs/g7_Daeh/fort.201")

quotients = []
for c in comps.values():
    c.fit()
    it = iter(c.fSFs.values())
    denom = next(it)
    num = next(it)
    quotients.append(num / denom)

# Draw
fig, axs = plt.subplots(2, 2, figsize=(11, 8))
for i, (k, c) in enumerate(comps.items()):
    ax: Axes = axs.flatten()[i]
    c.draw(ax, k)
    ax.annotate(
        f"iblock 5 / 0 : {quotients[i]:%.2uS}",
        xy=(0.75, 0.9),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=14,
    )
    if i == len(comps) - 1:
        ax.set_ylim(-2, 10)
        leg = ax.get_legend()
        if isinstance(leg, Legend):
            leg.set_loc("upper left")  # type: ignore

fig.tight_layout()
fig.savefig("./Outputs/fresco_step.png", dpi=200)
plt.show()
