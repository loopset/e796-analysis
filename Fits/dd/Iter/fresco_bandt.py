import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import numpy as np

data = {
    "(1,-1)": "./Inputs/fresco_bandt/1_-1/fort.202",
    "(1, -2)": "./Inputs/fresco_bandt/1_-2/fort.202",
    "(2, -2)": "./Inputs/fresco_bandt/2_-2/fort.202",
}

# Comparator
exp = phys.parse_txt("../Outputs/xs/g5_xs.dat", 3)
comp = phys.Comparator(exp)
for key, file in data.items():
    comp.add_model(key, file)

comp.fit()

fig, ax = plt.subplots(1, 1, figsize=(7,5))
ax : mplaxes.Axes
comp.draw(ax=ax)
leg = ax.get_legend()
leg.set_title("bandt (g.s, g5 @ 7.6 MeV)", prop={"size": 14})
fig.tight_layout()
fig.savefig("./Outputs/fresco_bandt.png", dpi=200)
plt.show()
