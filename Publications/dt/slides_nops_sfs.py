from collections import defaultdict

import pyphysics as phys
from pyphysics.actroot_interface import SFInterface
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from matplotlib.patches import Rectangle
import uncertainties as un
import uncertainties.unumpy as unp
from typing import List, Union, Dict
import sys

sys.path.append("../")
sys.path.append("./")
import styling as sty
import dt

df = dt.build_df()
sm = dt.build_sm()

# Parse NO PS sfs
nops = SFInterface("../../Fits/dt/Outputs/nops_sfs.root")
nops_sfs = []
nops_sm: phys.SMDataDict = defaultdict(list)
for i, name in enumerate(df["name"]):
    q = dt.assignments[name][1]
    val = next((e for e in nops.get(name) if e.fName == dt.equiv[q]), None)
    if val is None:
        continue
    sf = val.fSF
    sf.tag = "stat_sf"
    sf = dt.apply_systematics(sf)
    nops_sfs.append(sf)
    nops_sm[q].append(phys.ShellModelData(df.iloc[i]["ex"], sf))

df["nops_sf"] = nops_sfs
df["ratio"] = df["nops_sf"] / df["sf"]


###########################
fig, ax = plt.subplots(1, 1, figsize=(5, 3.25), constrained_layout=True)
ax: Axes
ax.errorbar(
    unp.nominal_values(df["ex"]),
    unp.nominal_values(df["ratio"]),
    yerr=unp.std_devs(df["ratio"]),
    ls="none",
    color="dodgerblue",
    **sty.errorbar_nols,
)
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel(r"No PS / With PS")
ax.axhline(1, ls="--", color="black")
o19 = phys.Particle("19O")
ax.axvline(o19.get_sn(), color="purple", **sty.styles["sn"], ls="dotted")
ax.annotate(
    rf"S$_{{\mathrm{{n}}}} =$ {o19.get_sn():.2f} MeV",
    xy=(o19.get_sn() + 3, 3.5),
    **sty.ann,
)
fig.savefig("./Outputs/slides_nops_0.png", dpi=300)


#################### ESPE calculation

# Binding energies
snadd = phys.Particle("21O").get_sn()
snrem = phys.Particle("20O").get_sn()

# Adding reactions: B. Fernández-Domínguez PRC 84 (2011)
add = phys.ShellModel()
add.data = {
    dt.qd52: [phys.ShellModelData(0, un.ufloat(0.34, 0.03))],
    dt.qs12: [phys.ShellModelData(un.ufloat(1.213, 0.007), un.ufloat(0.77, 0.09))],
    dt.qp12: [],
    dt.qp32: [],
}
qs = [dt.qd52, dt.qs12, dt.qp12, dt.qp32]


# Function to compute gaps
def compute(data: List[Union[phys.SMDataDict, phys.ShellModel]]):
    bars: List[phys.Barager] = []
    for i, removal in enumerate(data):
        b = phys.Barager()
        b.set_removal(removal, snrem)
        b.set_adding(add, snadd)
        b.do_for([dt.qd52, dt.qs12, dt.qp12, dt.qp32])
        bars.append(b)

    # Compute centroids
    centroids: List[Dict[phys.QuantumNumbers, Union[float, un.UFloat]]] = []
    for rem in data:
        aux = rem
        if isinstance(rem, phys.ShellModel):
            aux = rem.data
        centroids.append(dt.get_centroids(aux))  # type: ignore

    # Compute ESPES
    espes: List[Dict[phys.QuantumNumbers, Union[float, un.Variable]]] = []
    for bar in bars:
        dic = {}
        for q in qs:
            dic[q] = bar.get_ESPE(q)
        espes.append(dic)

    pairs = [(dt.qs12, dt.qd52), (dt.qd52, dt.qp12), (dt.qp12, dt.qp32)]
    gaps: List[List[Union[float, un.Variable]]] = []
    # Compute gaps
    for bar in bars:
        lis = []
        for top, bottom in pairs:
            gap = bar.get_gap(top, bottom)
            lis.append(gap)
        gaps.append(lis)

    return (espes, gaps)


espes = compute([sm])
nops_espes = compute([nops_sm])

DictType = Dict[phys.QuantumNumbers, float | un.Variable]


def plot(ax: Axes, data: DictType, idx: int, qargs: List | None = None, **kwargs):
    # Some options
    width = 0.7
    qs = [dt.qs12, dt.qd52, dt.qp12, dt.qp32]
    if qargs is not None:
        qs = qargs
    for q in qs:
        espe = data.get(q, None)
        if espe is None:
            continue
        y = un.nominal_value(espe)
        uy = un.std_dev(espe)
        # Color
        color = sty.barplot[q]["ec"]
        # Line
        ax.plot(
            [idx - width / 2, idx + width / 2],
            [y] * 2,
            lw=1.25,
            color=color,
            label=f"{q.format()}" if idx == 0 else None,
            **kwargs,
        )
        # Rectangle patch
        if uy > 0:
            rec = Rectangle(
                xy=(idx - width / 2, y - uy),
                width=width,
                height=2 * uy,
                ec="none",
                fc=color,
                alpha=0.25,
            )
            ax.add_patch(rec)


fig, ax = plt.subplots(1, 1, figsize=(5, 3.25), constrained_layout=True)
ax: Axes
# Standard
plot(ax=ax, data=espes[0][0], idx=0)
# Modified
plot(ax=ax, data=nops_espes[0][0], idx=1)

# Magic numbers
qs = [dt.qs12, dt.qd52, dt.qp12, dt.qp32]
magic = [14, 8, 6]
for i, q in enumerate(qs):
    color = sty.barplot[q]["ec"]
    y = un.nominal_value(espes[0][0][q])
    ax.annotate(
        f"{q.format()}",
        xy=(-0.6, y),
        ha="center",
        va="center",
        fontsize=14,
        color=color,
    )
    if i < 3:
        yc = (y + un.nominal_value(espes[0][0][qs[i + 1]])) / 2
        ax.annotate(
            f"N = {magic[i]}", xy=(-1.05, yc), ha="center", va="center", fontsize=12
        )

# Gap values
pairs = [(dt.qs12, dt.qd52), (dt.qd52, dt.qp12), (dt.qp12, dt.qp32)]

for i, espes in enumerate([espes[0][0], nops_espes[0][0]]):
    for j, pair in enumerate(pairs):
        g = espes[pair[0]] - espes[pair[1]]  # type: ignore
        x = i
        y = un.nominal_value(espes[pair[0]] + espes[pair[1]]) / 2  # type: ignore
        fmt = ""
        if hasattr(g, "nominal_value"):
            fmt = f"{g:.1uS}"
        else:
            fmt = f"{g:.1f}"
        ax.annotate(fmt, xy=(x, y), ha="center", va="center", fontsize=12)

# Axis settings
ax.set_xlim(-1.4)
labels = ["With PS", "No PS"]
ax.set_xticks(list(range(len(labels))), labels)
ax.tick_params(axis="x", which="both", bottom=False, top=False)
ax.set_ylim(-25, 0)
ax.set_ylabel(r"$\nu$ ESPE [MeV]")
fig.savefig("./Outputs/slides_nops_1.png", dpi=300)


plt.show()
