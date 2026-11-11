import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from typing import List

# Read data
nuclei = ["O15", "O16", "O17"]
sms = []
for nucleus in nuclei:
    file = f"/media/Data/E796v2/kshell/0hw/summary_{nucleus}_sfotls_modtnew.txt"
    sm = phys.ShellModel()
    sm.add_summary(file)
    sms.append(sm)


def build_exp():
    o16 = phys.Particle("16O")
    o15 = phys.Particle("15O")
    o17 = phys.Particle("17O")
    sms = [phys.ShellModel() for _ in range(3)]
    # Minus
    sms[0].data[phys.Orbital(1, -1, -0.5, 0.5)] = [
        phys.ShellModelData(0, 0, phys.StateJpi(0.5, -1), phys.StateJpi(), -o15.get_be())
    ]
    # Ref
    sms[1].data[phys.Orbital(1, -1, 0, 0)] = [
        phys.ShellModelData(0, 0, phys.StateJpi(0, +1), phys.StateJpi(), -o16.get_be())
    ]
    # Plus
    sms[2].data[phys.Orbital(1, -1, 2.5, 0.5)] = [
        phys.ShellModelData(0, 0, phys.StateJpi(2.5, +1), phys.StateJpi(), -o17.get_be())
    ]
    return sms


exp = build_exp()


def plot_bars(sms: List[phys.ShellModel], idx: int, ax: Axes):
    states = [-1.5, -0.5, +2.5, +0.5, +1.5]
    labels = ["$3/2^-$", "$1/2^-$", "$5/2^+$", "$1/2^+$", "$3/2^+$"]
    colors = ["grey", "darkgreen", "dodgerblue", "crimson", "darkorange"]
    bes = {}
    # Get reference BE (16O)
    ref = sms[1].data[phys.Orbital(1, -1, 0, 0)][0].BE
    # Minus (15O)
    for state in states:
        data = sms[0].data.get(phys.Orbital(1, -1, state, 0.5), None)
        if data is not None:
            be = data[0].BE
            bes[state] = ref - be  # type: ignore
    # Plus (17O)
    for state in states:
        data = sms[2].data.get(phys.Orbital(1, -1, state, 0.5), None)
        if data is not None:
            be = data[0].BE
            bes[state] = be - ref  # type: ignore
    print(f"====== {idx} ======")
    for state, be in zip(states, bes):
        print(f"  {state:+.1f} : {be:.3f} MeV")
    print(f"  Ref : {ref:.3f} MeV")
    print("====================")

    width = 0.3
    offset = 0.075
    for i, state in enumerate(states):
        be = bes.get(state, np.nan)
        ax.plot([idx - width, idx + width], [be, be], color=colors[i], lw=1.25)
        ax.annotate(
            f"{labels[i]}",
            xy=(idx + width + offset, be),
            ha="center",
            va="center",
            fontsize=12,
            color=colors[i],
        )

    return


fig, ax = plt.subplots()

# Exp
plot_bars(exp, 0, ax)

# SFO-tlsnew
plot_bars(sms, 1, ax)

ax.set_xlim(-0.5, 1.5)
plt.show()
