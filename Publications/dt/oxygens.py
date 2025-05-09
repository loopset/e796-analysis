import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.collections as mplcol
import matplotlib.patches as mplpat
import uncertainties as un
import uncertainties.unumpy as unp
import ROOT as r
from typing import List, Dict

# Quantum numbers
q52 = phys.QuantumNumbers(0, 2, 2.5)
q12 = phys.QuantumNumbers(0, 1, 0.5)


def init(ex: float | un.UFloat, sf: float | un.UFloat) -> phys.ShellModelData:
    return phys.ShellModelData(ex, sf)


######################### 19O
bo19 = phys.Barager.read("./Inputs/dt_barager.pkl")


######################### 17O
## 18O(d,t)
o17_rem = {
    q52: [init(0, 1.53)],
    q12: [init(3.055, 1.08), init(5.935, 0.06), init(9.160, 0.10), init(11.082, 0.96)],
}

## 16O(d,p)
o17_add = {
    q52: [init(0, un.ufloat(0.84, 0.04))],
    q12: [
        init(un.ufloat(3.05498, 0.00020), 0.032),
    ],
}
bo17 = phys.Barager()
bo17.set_removal(o17_rem, r.ActPhysics.Particle("18O").GetSn())  # type: ignore
bo17.set_adding(o17_add, r.ActPhysics.Particle("19O").GetSn())  # type: ignore
bo17.do_for([q52, q12])


######################### 15O
## 16O(d,t)
## No data on this...


# Plot
def plot(ax: mplaxes.Axes, x: List[str], y: List[un.UFloat], **kwargs) -> None:
    width = 0.5
    boxes = []
    for i in range(len(x)):
        if "label" in kwargs and i > 0:
            kwargs["label"] = "_" + kwargs["label"]
        yval = un.nominal_value(y[i])
        yerr = un.std_dev(y[i])
        ax.plot([i - width / 2, i + width / 2], [yval] * 2, ls="-", lw=1.5, **kwargs)
        box = mplpat.Rectangle(
            (i - width / 2, yval - yerr), width=width, height=2 * yerr
        )
        boxes.append(box)
    boxcol = mplcol.PatchCollection(
        boxes, facecolor=kwargs.get("color"), alpha=0.5, edgecolor="none"
    )
    ax.add_collection(boxcol)
    ax.tick_params(axis="x", which="major", pad=10)
    ax.set_xticks(range(len(x)), x)


fig, ax = plt.subplots(1, 1, figsize=(7, 5))
ax: mplaxes.Axes
labels = [r"$^{17}$O", r"$^{19}$O"]
plot(
    ax,
    labels,
    [bo17.get_ESPE(q52), bo19.get_ESPE(q52)],
    color="dodgerblue",
    label=q52.format(),
)
plot(ax, labels, [bo17.get_ESPE(q12), bo19.get_ESPE(q12)], color="green", label=q12.format())
ax.legend()
ax.axhline(0, color="grey", ls="--")
ax.set_ylabel("ESPE [MeV]")
ax.set_ylim(-20, 5)

fig.tight_layout()
fig.savefig("./Outputs/oxygens.pdf")
plt.show()
