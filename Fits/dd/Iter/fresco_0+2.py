import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes

# Read experimental data (g7)
exp = phys.parse_txt("../Outputs/xs/g7_xs.dat", 3)

# Read theoretical predictions
comp = phys.Comparator(exp)

theo = {
    "gs": "../Inputs/g7_Daeh/fort.201",
    r"$2^+_1$": "./Inputs/fresco_0+2/2+1/fort.205",
    r"$2^+_1\; 2^+_2$": "./Inputs/fresco_0+2/2+1_2+2/fort.205",
    r"$2^+_1\; 3^-_1$": "./Inputs/fresco_0+2/2+1_3-1/fort.205",
    r"$2^+_2\; 3^-_1$": "./Inputs/fresco_0+2/2+2_3-1/fort.205",
    r"$3^-_1$": "./Inputs/fresco_0+2/3-1/fort.205",
    r"$2^+_1\;2^+_2\; 3^-_1$": "./Inputs/fresco_0+2/2+1_2+2_3-1/fort.205",
}

for k, v in theo.items():
    comp.add_model(k, v)

comp.fit()

# Draw
fig, ax = plt.subplots(figsize=(7, 5))  # type: ignore
ax: Axes
comp.draw(ax, "g7 @ 9.6 MeV 0+")
ax.set_ylim(0, 7)

fig.tight_layout()
fig.savefig("./Outputs/fresco_0+2.png", dpi=200)
plt.show()
