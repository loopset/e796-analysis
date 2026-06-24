import pyphysics as phys
import matplotlib.pyplot as plt
import numpy as np

exp = phys.parse_txt("../Outputs/xs/g0_xs.dat", 3)

comp = phys.Comparator(exp)
comp.add_model("CH89", "../Inputs/g0_CH89/fort.201")
comp.add_model("CH89 + SFRESCO", "./fort.201")
comp.fit()

fig, ax = plt.subplots(1, 1, figsize=(5, 4), constrained_layout=True)
ax.set_yscale("log")
comp.draw(ax=ax)

ax.set_ylim(4e1, 4e3)
ax.set_xlim(14, 28)

plt.savefig("./sfresco.png", dpi=300)

plt.show()
