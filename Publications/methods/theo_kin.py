import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from matplotlib.lines import Line2D

import sys

sys.path.append("../")
import styling as sty

channels = ["(p,p)", "(p,d)", "(d,d)", "(d,t)"]
exs = [0, 5]
colors = ["C0", "C3" ,"C1", "C2"]
ls = ["solid", "dashed"]

fig, ax = plt.subplots(figsize=(4.5, 3.5), constrained_layout=True)
ax: Axes
for i, channel in enumerate(channels):
    for j, ex in enumerate(exs):
        kin = phys.Kinematics(f"20O{channel}@700|{ex}").get_line3()
        aux = channel
        if ex > 0:
            aux = "_"
        ax.plot(kin[0], kin[1], color=colors[i], ls=ls[j], label=aux)

leg = ax.legend(loc=(0.025, 0.425))
ax.add_artist(leg)

ax.legend(
    handles=[
        Line2D([], [], color="black", label="g.s."),
        Line2D([], [], color="black", ls="--", label=r"5 MeV"),
    ],
    # loc=(0.025, 0.25),
    handlelength=1.5,
    handletextpad=0.5,
    loc="upper right",
    title=r"E$_x$"
)
ax.annotate(r"$^{20}$O", xy=(0.1, 0.835), xycoords="axes fraction", **sty.ann)
ax.set_xlim(0, 95)
ax.set_ylim(0, 50)
ax.set_xlabel(r"$\theta_{lab}$ [$\circ$]")
ax.set_ylabel(r"E$_{lab}$ [MeV]")

fig.savefig(sty.thesis + "theo_kin.pdf", dpi=300)
plt.show()
