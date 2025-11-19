import pyphysics as phys
from pyphysics.actroot_interface import FitInterface
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as un
import sys

sys.path.append("../")

import styling as sty

# Read theoretical
sfo = phys.ShellModel()
sfo.add_summary("../../Fits/dd/Inputs/SM/summary_O20_sfotls_mod.txt")
sfo.set_max_Ex(10)

# Read experimental
fit = FitInterface("../../Fits/dd/Outputs/fit_juan_RPx.root")
exp = phys.ShellModel()
mapping = {
    "g1": phys.QuantumNumbers(1, -1, 2),
    "g2": phys.QuantumNumbers(2, -1, 2),
    "g3": phys.QuantumNumbers(1, -1, -3),
    "g4": phys.QuantumNumbers(-11, -1, 11),
    "v0": phys.QuantumNumbers(2, -1, -3),
    "v1": phys.QuantumNumbers(3, -1, +2),
}


# Gate on states
theos = []
qs = [+2, -3]
for q in qs:
    aux = phys.ShellModel()
    for key, vals in sfo.data.items():
        if key.j == q:
            aux.data.update({key: vals})  # type: ignore
    theos.append(aux)


def plot_bars(ax: mplaxes.Axes, idx: int, sm: phys.ShellModel) -> None:
    width = 0.75
    for q, val in sm.data.items():
        if not len(val):
            continue
        l = idx - width / 2
        r = idx + width / 2
        y = un.nominal_value(val[0].Ex)
        color = f"C{int(abs(q.j)) - 1}"
        if q.j == 0:
            color = "C8"
        ax.plot([l, r], [y] * 2, color=color)
        ax.annotate(q.format(), xy=(r + 0.05, y), ha="center", va="center", fontsize=12)


for key, q in mapping.items():
    exp.data.update({q: [phys.ShellModelData(fit.get(key)[0], -1)]})  # type: ignore

fig, ax = plt.subplots()
plot_bars(ax, 1, exp)

for theo in theos:
    plot_bars(ax, 2, theo)


# Tick parameters
ax.set_xticks([1, 2], ["Exp", "SFO-tls"])
ax.tick_params(axis="x", which="both", bottom=False, top=False, pad=15)
ax.tick_params(axis="y", which="both", right=False)
for spine in ["bottom", "top", "right"]:
    ax.spines[spine].set_visible(False)
ax.set_xlim(0.5, 2.5)
ax.set_ylim(0, 10)
ax.set_ylabel(r"E$_{\text{x}}$ [MeV]")

fig.tight_layout()
fig.savefig(sty.thesis + "dd_ine_sfotls.pdf", dpi=300)
plt.show()
