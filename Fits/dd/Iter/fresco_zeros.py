import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from matplotlib.legend import Legend

# g7 state now
exp = phys.parse_txt("../Outputs/xs/g7_xs.dat", 3)

# Fresco calculations
fresco = {
    "g7 (1,)": "./Inputs/fresco_zeros/1/fort.201",
    "g7 (2,)": "./Inputs/fresco_zeros/2/fort.201",
    "g7 (1,1)": "./Inputs/fresco_zeros/1_1/fort.202",
    r"\textbf{gs} (1,1)": "./Inputs/fresco_zeros/1_1/fort.201",
    "g7 (1,2)": "./Inputs/fresco_zeros/1_2/fort.202",
}

# Comparator
comp = phys.Comparator(exp)
for k, v in fresco.items():
    comp.add_model(k, v)
comp.fit()

# Draw
fig, ax = plt.subplots(figsize=(7, 5))
if isinstance(ax, Axes):
    comp.draw(ax, "g7 @ 9.6 MeV")
    ax.set_ylim(-2, 10)
    leg = ax.get_legend()
    if isinstance(leg, Legend):
        leg.set_title("State bandt (fort.201, fort.202)", prop=dict(size=14))
fig.tight_layout()
fig.savefig("./Outputs/fresco_zeros.png", dpi=200)
plt.show()
