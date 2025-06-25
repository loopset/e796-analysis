from collections import defaultdict
import pickle
from typing import Dict, List, Union

from matplotlib import hatch
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as un
import uncertainties.unumpy as unp
import numpy as np

import glob
import os
import re

files = glob.glob("./Inputs/exp/*.txt")
pattern = re.compile(r"ex_(\d+)_(\d{2})\.txt$")

comps: List[phys.Comparator] = []
exs: List[float] = []

for file in files:
    name = os.path.basename(file)
    match = pattern.search(name)
    if match:
        ex = float(f"{match.group(1)}.{match.group(2)}")
    else:
        ex = 0
    exs.append(ex)
    # Parse file
    data = phys.parse_txt(file)
    comp = phys.Comparator(data)
    comps.append(comp)
# Sort by increasing ex
sorted_ex = sorted(zip(exs, comps), key=lambda x: x[0])
exs[:], comps[:] = zip(*sorted_ex)
# Add theoretical predictions and fit
for i, comp in enumerate(comps):
    comp.add_model("mairle", f"./Inputs/DaehPang/fort.2{i + 2:02d}")
    comp.fit()

# Add manual states
exs_man = [11.41, 12.12, 12.76]
sfs_man = [0.04, 0.24, 0.17]
for ex, sf in zip(exs_man, sfs_man):
    exs.append(ex)
    # Fake data
    n = 20
    x = np.linspace(0, 40, n)
    y = np.full(n, 1)
    comp = phys.Comparator(np.column_stack((x, y)))
    comp.add_model("mairle", "./Inputs/DaehPang/fort.202")
    comp.fit()
    comp.fSFs["mairle"] = sf
    comps.append(comp)

# Add labels
qd52 = phys.QuantumNumbers(0, 2, 2.5)
qd32 = phys.QuantumNumbers(0, 2, 1.5)
qs12 = phys.QuantumNumbers(1, 0, 0.5)
qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
assignments = [qd52, qs12, qp12, None, qp32, qd32, qp32, qp12, qp32, qp32, qp12]
assignments_manual = [qp32, qp32, qp32]
assignments.extend(assignments_manual)

# Our reanalysis
reana: phys.SMDataDict = defaultdict(list)
for i in range(len(exs)):
    q = assignments[i]
    if q is None:
        continue
    ex = exs[i]
    sf = comps[i].get_sf("mairle")
    reana[q].append(phys.ShellModelData(ex, sf))
with open("./Outputs/reanalysis.pkl", "wb") as f:
    pickle.dump(reana, f)

# Mairle's original results
sfs_mairle = [1.53, 0.21, 1.08, np.nan, 0.12, 0.10, 0.53, 0.06, 0.15, 0.10, 0.10, 0.04, 0.24, 0.17]
mairle: phys.SMDataDict = defaultdict(list)
for i in range(len(exs)):
    q = assignments[i]
    if q is None:
        continue
    ex = exs[i]
    sf = sfs_mairle[i]
    mairle[q].append(phys.ShellModelData(ex, sf))
with open("./Outputs/mairle.pkl", "wb") as f:
    pickle.dump(mairle, f)

# Draw
figs = []
imax = 6
for i, (ex, comp) in enumerate(zip(exs, comps)):
    if i % imax == 0:
        fig, axs = plt.subplots(2, 3, figsize=(12, 8))
        figs.append(fig)
    idx = i % imax
    ax: mplaxes.Axes = axs.flat[idx]  # type: ignore
    comp.draw(ax=ax)
    ax.set_title(
        f"Ex = {ex:.2f} MeV"
        + (f" {assignments[i].format()}" if assignments[i] is not None else "")
    )
    ax.set_yscale("log")

# Comparison
fig, axs = plt.subplots(2, 2, figsize=(12, 8))
figs.append(fig)
ax: mplaxes.Axes = axs[0, 0]

# Styling
sty = {
    qd52: {"marker": "s", "color": "dodgerblue", "hatch": ".."},
    qs12: {"marker": "o", "color": "crimson", "hatch": r"\\"},
    qp12: {"marker": "h", "color": "green", "hatch": r"--"},
    qp32: {"marker": "h", "color": "grey", "hatch": "//"},
    qd32: {"marker": "x", "color": "lightskyblue", "hatch": "oo"},
}
# C2S values
for i, dictio in enumerate([mairle, reana]):
    for q, vals in dictio.items():
        for j, val in enumerate(vals):
            ax.errorbar(
                unp.nominal_values(val.Ex),
                unp.nominal_values(val.SF),
                yerr=unp.std_devs(val.SF),
                marker=(sty[q]["marker"]) if q in sty else "x",
                color=(sty[q]["color"]) if q in sty else "black",
                alpha=0.5 if i == 0 else 1,
                label=q.format() if i == 1 and j == 0 else None,
            )
ax.legend()
ax.set_xlabel("Ex [MeV]")
ax.set_ylabel("C2S")

# Strengths
DictType = Dict[phys.QuantumNumbers, Union[float, un.UFloat]]
sts: List[DictType] = []
cents: List[DictType] = []
for i, dictio in enumerate([mairle, reana]):
    stdict: DictType = defaultdict(float)
    centdict: DictType = defaultdict(float)
    for q, vals in dictio.items():
        st = 0
        for val in vals:
            st += val.SF  # type: ignore
        stdict[q] = st
        cent = 0
        for val in vals:
            cent += val.SF * val.Ex / st  # type: ignore
        centdict[q] = cent
    sts.append(stdict)
    cents.append(centdict)


ax = axs[0, 1]
for i, dictio in enumerate(sts):
    ax.errorbar(
        [q.format() for q in dictio.keys()],
        [un.nominal_value(st) for st in dictio.values()],
        yerr=[un.std_dev(st) for st in dictio.values()],
        marker="s",
        color="dodgerblue" if i == 0 else "crimson",
        label="Mairle" if i == 0 else "Reanalysis",
    )
ax.legend()

ax = axs[1, 0]
for i, dictio in enumerate(cents):
    for q, cent in dictio.items():
        ax.bar(
            un.nominal_value(cent),
            height=un.nominal_value(sts[i][q]),
            yerr=un.std_dev(sts[i][q]),
            color="none",
            edgecolor=(sty[q]["color"]) if q in sty else "black",
            hatch=sty[q]["hatch"] if q in sty else None,
            alpha=0.5 if i == 0 else 1,
            label=q.format() if i == 1 else None,
        )
ax.legend(loc="upper right")
ax.set_ylim(0, 2)


for fig in figs:
    fig.tight_layout()
figs[-1].savefig("./Outputs/mairle_reana.png")

plt.show()
