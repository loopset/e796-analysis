import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from typing import List


def build_theo(dir: str) -> List[phys.ShellModel]:
    nuclei = ["O15", "O16", "O17"]
    sms = []
    for nucleus in nuclei:
        file = f"/media/Data/E796v2/kshell/{dir}/summary_{nucleus}_sfotls_modtnew.txt"
        sm = phys.ShellModel()
        sm.add_summary(file)
        sms.append(sm)
    return sms


def build_exp():
    o16 = phys.Particle("16O")
    o15 = phys.Particle("15O")
    o17 = phys.Particle("17O")
    sms = [phys.ShellModel() for _ in range(3)]
    # Minus
    # 1/2- g.s.
    sms[0].data[phys.Orbital(1, -1, -0.5, 0.5)] = [
        phys.ShellModelData(
            0, 0, phys.StateJpi(0.5, -1), phys.StateJpi(), -o15.get_be()
        )
    ]
    # 3/2- 1st
    sms[0].data[phys.Orbital(1, -1, -1.5, 0.5)] = [
        phys.ShellModelData(
            0, 0, phys.StateJpi(1.5, -1), phys.StateJpi(), -o15.get_be() + 6.176
        )
    ]
    # Ref
    sms[1].data[phys.Orbital(1, -1, 0, 0)] = [
        phys.ShellModelData(0, 0, phys.StateJpi(0, +1), phys.StateJpi(), -o16.get_be())
    ]
    # Plus
    # 5/2+ g.s.
    sms[2].data[phys.Orbital(1, -1, 2.5, 0.5)] = [
        phys.ShellModelData(
            0, 0, phys.StateJpi(2.5, +1), phys.StateJpi(), -o17.get_be()
        )
    ]
    # 1/2+ 1st
    sms[2].data[phys.Orbital(1, -1, 0.5, 0.5)] = [
        phys.ShellModelData(
            0, 0, phys.StateJpi(0.5, +1), phys.StateJpi(), -o17.get_be() + 0.870
        )
    ]
    # 3/2+
    sms[2].data[phys.Orbital(1, -1, 1.5, 0.5)] = [
        phys.ShellModelData(
            0, 0, phys.StateJpi(1.5, +1), phys.StateJpi(), -o17.get_be() + 5.087
        )
    ]
    return sms


# Experiment
exp = build_exp()

# Theoretical
hw0 = build_theo("0hw")
hw2 = build_theo("2hw")
hw4 = build_theo("4hw")
it0hw0 = build_theo("it0/0hw")
it1hw0 = build_theo("it1/0hw")
it2hw0 = build_theo("it2/0hw")
it3hw0 = build_theo("it3/0hw")
it4hw2 = build_theo("it4/2hw")
it5hw2 = build_theo("it5/2hw")
it6hw2 = build_theo("it6/2hw")
# Suzuki's recommendations
itsuz0hw0 = build_theo("itsuzuki/0hw")
itsuz0hw4 = build_theo("itsuzuki/4hw")
# Suzuki + SPE changs
itsuz0spehw0 = build_theo("itsuzuki_spe/0hw")
itsuz0spehw4 = build_theo("itsuzuki_spe/4hw")
# But 2hw
itsuz0spehw2 = build_theo("itsuzuki_spe/2hw")


def plot_bars(sms: List[phys.ShellModel], idx: int, ax: Axes, label: str = ""):
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
    # Gap
    gap = bes[+2.5] - bes[-0.5]
    ygap = (bes[+2.5] + bes[-0.5]) / 2
    print(f"====== {idx} : {label} ======")
    for state in states:
        if state in bes:
            print(f"  {state:+.1f} : {bes[state]:.3f} MeV")
    print(f"  Ref : {ref:.3f} MeV")
    print(f"  Gap : {gap:.3f} MeV")
    print("====================")

    width = 0.25
    offset = 0.1
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
    ax.annotate(
        rf"$\Delta = ${gap:.2f} MeV",
        xy=(idx, ygap),
        ha="center",
        va="center",
        fontsize=10,
    )

    return


sms = [
    exp,
    hw0,
    hw2,
    hw4,
    itsuz0hw0,
    itsuz0spehw2,
    itsuz0hw4,
    # itsuz0spehw0,
    # itsuz0spehw4,
    # it0hw0,
    # it1hw0,
    # it2hw0,
    # it3hw0,
    # it4hw2,
    # it5hw2,
    # it6hw2,
]
labels = [
    "Exp",
    "0hw",
    "2hw",
    "4hw",
    "0hw-1.15-1.15[+2.8]",
    "2hw-1.15-1.15[+2.8]",
    "4hw-0.5-0.5",
    # "0hw-0.5-0.5[+2.0]",
    # "4hw-0.5-0.5[+2.0]",
    # "0hw-0.7+0",
    # "0hw+0.7+0",
    # "0hw+0-10",
    # "0hw+0+10",
    # "2hw+0-10",
    # "2hw+0+10",
    # "2hw+1.2-5",
]

fig, ax = plt.subplots(figsize=(14, 6), constrained_layout=True)
for i, sm in enumerate(sms):
    plot_bars(sm, i, ax, label=labels[i])

# Plot experimental
ax.axhline(-15.664, color="darkgreen", ls="--", alpha=0.5)
ax.axhline(-4.143, color="dodgerblue", ls="--", alpha=0.5)

ax.set_xlim(-0.5, len(sms) - 0.5)
ax.set_xticks(range(len(sms)), labels=labels, fontsize=14)
ax.tick_params(axis="x", which="minor", bottom=False, top=False)
ax.set_ylabel("E [MeV]")

fig.suptitle(r"$^{15,16,17}O$", fontsize=16, fontweight="bold")

fig.savefig("./Outputs/kshell_n8.pdf", dpi=300)

plt.show()
