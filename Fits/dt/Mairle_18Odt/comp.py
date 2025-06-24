from collections import defaultdict
import pickle
from typing import Dict, List
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
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

# Add labels
qd52 = phys.QuantumNumbers(0, 2, 2.5)
qd32 = phys.QuantumNumbers(0, 2, 1.5)
qs12 = phys.QuantumNumbers(1, 0, 0.5)
qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
assignments = [qd52, qs12, qp12, None, qp32, qd32, qp32, qp12, qp32, qp32, qp12]

# Add theoretical predictions and fit
for i, comp in enumerate(comps):
    comp.add_model("mairle", f"./Inputs/DaehPang/fort.2{i + 2:02d}")
    comp.fit()

# Save to file
out: phys.SMDataDict = defaultdict(list)
for i in range(len(exs)):
    q = assignments[i]
    if q is None:
        continue
    ex = exs[i]
    sf = comps[i].get_sf("mairle")
    out[q].append(phys.ShellModelData(ex, sf))
with open("./Outputs/18O_dt.pkl", "wb") as f:
    pickle.dump(out, f)

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
mairle = [1.53, 0.21, 1.08, np.nan, 0.12, 0.10, 0.53, 0.06, 0.15, 0.10, 0.10]
reana = []
for ass, comp in zip(assignments, comps):
    if ass is None:
        sf = np.nan
    else:
        sf = comp.get_sf("mairle")
    reana.append(sf)

fig, ax = plt.subplots()
figs.append(fig)
ax: mplaxes.Axes
for y, label in zip([mairle, reana], ["Mairle", "Reanalysis"]):
    ax.errorbar(
        exs, unp.nominal_values(y), yerr=unp.std_devs(y), marker="s", label=label
    )
ax.legend()
ax.set_xlabel("Ex [MeV]")
ax.set_ylabel("C2S")
for fig in figs:
    fig.tight_layout()
figs[-1].savefig("./Outputs/mairle_reana.png")

plt.show()
