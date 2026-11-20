import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
import uncertainties as un

files = {
    "3,2hw-1.15+0.10[+2.8]": "../../kshell/20O/plus_0.1/summary_O20_sfotls_modtnew.txt",
    "1,2hw-1.15-0.50[+2.8]": "../../kshell/20O/minus_0.5/summary_O20_sfotls_modtnew.txt",
    "1,2hw-1.15-0.00[+2.8]": "../../kshell/20O/zero/summary_O20_sfotls_modtnew.txt",
}

# Build experimental
exp = phys.ShellModel()
exp.data[phys.Orbital(1, -1, 0, 0)] = [
    phys.ShellModelData(0, 0, phys.StateJpi(0, +1)),
    phys.ShellModelData(1.67, 0, phys.StateJpi(2, +1)),
    phys.ShellModelData(3.57, 0, phys.StateJpi(4, +1)),
    phys.ShellModelData(4.07, 0, phys.StateJpi(2, +1)),
    phys.ShellModelData(4.46, 0, phys.StateJpi(0, +1)),
    # Negative parity
    phys.ShellModelData(5.35, 0, phys.StateJpi(1, -1)),
    phys.ShellModelData(5.61, 0, phys.StateJpi(3, -1)),
]


sfos = {"Exp.": exp}
for key, file in files.items():
    sm = phys.ShellModel()
    sm.add_summary(file)
    sfos[key] = sm


def plot_bars(sm: phys.ShellModel, idx: int, ax: Axes, label: str = ""):
    width = 0.25
    offset = 0.1
    for key, vals in sm.data.items():
        for val in vals:
            y = un.nominal_value(val.Ex)
            jpii = val.Jpii
            color = "dodgerblue" if jpii.pi == 1 else "crimson"
            ax.plot([idx - width, idx + width], [y, y], color=color, lw=1.25)
            ax.annotate(
                f"{jpii.format()}",
                xy=(idx + width + offset, y),
                ha="center",
                va="center",
                fontsize=12,
            )
    return


fig, ax = plt.subplots(constrained_layout=True, figsize=(8, 4))
for idx, (label, sm) in enumerate(sfos.items()):
    plot_bars(sm, idx, ax, label=label)

# Horizontal lines for references
ax.axhline(1.67, color="dodgerblue", ls="--", lw=1, alpha=0.5)
ax.axhline(3.57, color="dodgerblue", ls="--", lw=1, alpha=0.5)

ax.set_ylim(-0.2, 8)
ax.set_xlim(-0.5, len(sfos) - 0.5)
ax.set_xticks(range(len(sfos)), labels=list(sfos.keys()), fontsize=14)
ax.tick_params(axis="x", which="minor", bottom=False, top=False)
ax.set_ylabel("E [MeV]")
fig.suptitle(r"$^{20}O$", fontsize=16, fontweight="bold")

fig.savefig("./Outputs/kshell_20O.pdf", dpi=300)
plt.show()
