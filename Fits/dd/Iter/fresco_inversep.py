import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from matplotlib.legend import Legend

exp = phys.parse_txt("../Outputs/xs/g5_xs.dat", 3)

comp = phys.Comparator(exp)
data = {
    "3-: (1,-1)": "./Inputs/fresco_inversep/1_-1/fort.202",
    "3+: (-1,1)": "./Inputs/fresco_inversep/-1_1/fort.202",
    "3+: (1,1)": "./Inputs/fresco_inversep/1_1/fort.202",
}
for k, v in data.items():
    comp.add_model(k, v)
comp.fit()

fig, ax = plt.subplots(figsize=(7,5))
ax: Axes
comp.draw(ax)
leg = ax.get_legend()
if isinstance(leg, Legend):
    leg.set_title(r"$3^{\pi}$ bandt (gs, g5)", prop=dict(size=14))
    leg.set_draggable(True)
fig.tight_layout()
fig.savefig("./Outputs/fresco_inversep.png", dpi=200)
plt.show()

