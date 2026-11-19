import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
import uncertainties as un

sfo = phys.ShellModel()
sfo.add_summary("../../kshell/itsuzuki_spe/20O/summary_O20_sfotls_modtnew.txt")


def plot_bars(sm: phys.ShellModel, idx: int, ax: Axes, label: str = ""):
    states = [-1.5, -0.5, +2.5, +0.5, +1.5]
    labels = ["$3/2^-$", "$1/2^-$", "$5/2^+$", "$1/2^+$", "$3/2^+$"]
    colors = ["grey", "darkgreen", "dodgerblue", "crimson", "darkorange"]

    width = 0.25
    offset = 0.1
    for key, vals in sm.data.items():
        for val in vals:
            y = un.nominal_value(val.Ex)
            ax.plot([idx - width, idx + width], [y, y], color="black", lw=1.25)
            ax.annotate(
                f"{val.Jpii.format()}",
                xy=(idx + width + offset, y),
                ha="center",
                va="center",
                fontsize=12,
            )
    return

fig, ax = plt.subplots(constrained_layout=True, figsize=(6, 4))
plot_bars(sfo, 0, ax, label="SFO")

ax.set_ylim(0, 8)
ax.set_xlim(-0.5, 0.75)

plt.show()
