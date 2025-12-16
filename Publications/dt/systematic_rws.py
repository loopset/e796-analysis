from collections import defaultdict

import matplotlib as mpl
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
from matplotlib.patches import Patch
import uncertainties as un
import numpy as np
from pathlib import Path
import re

import sys

sys.path.append("../")
sys.path.append("./")
import styling as sty
import dt

exps = {
    "g0": phys.parse_txt("../../Fits/dt/Outputs/xs/g0_xs.dat", 3),
    "g1": phys.parse_txt("../../Fits/dt/Outputs/xs/g1_xs.dat", 3),
    "g2": phys.parse_txt("../../Fits/dt/Outputs/xs/g2_xs.dat", 3),
}

# which fresco files correspond to each state
which = {"g0": 204, "g1": 202, "g2": 203}

# Labels
labels = {"g0": "g.s (L = 2)", "g1": r"1$^{st}$ (L = 0)", "g2": r"2$^{nd}$ (L = 1)"}

# read iter files
regexp = re.compile(r"^r_ws_([+-]?\d+(?:\.\d+)?)$")
files = defaultdict(dict)
for key in exps.keys():
    base = f"../../Fits/dt/Macros/Inputs/{key}/"
    dirs = []
    for p in Path(base).iterdir():
        if not p.is_dir():
            continue
        match = regexp.match(p.name)
        if not match:
            continue
        rws = float(match.group(1))
        # fresco file
        fresco = base + p.name + f"/fort.{which[key]}"
        # compare
        comp = phys.Comparator(exps[key])
        comp.add_model("rws", fresco)
        comp.fit()
        # add
        files[key][rws] = comp.fSFs["rws"]

# sort by incresing radius
for k in files:
    files[k] = dict(sorted(files[k].items(), key=lambda t: t[0]))

fig, ax = plt.subplots(figsize=(7, 3.5))
ax: mplaxes.Axes
for key, inner in files.items():
    ax.errorbar(
        [k for k in inner.keys()],
        [un.nominal_value(v) for v in inner.values()],
        yerr=[un.std_dev(v) for v in inner.values()],
        label=labels[key],
        **sty.errorbar_line,
    )
ax.axvline(1.25, color="crimson", ls="--", label="Current")
ax.legend()

ax.set_xlabel(r"$r_{WS}$ [fm]")
ax.set_ylabel(r"$C^2S$")
fig.tight_layout()
fig.savefig(sty.thesis + "systematic_rws.pdf", dpi=300)

# Another sort of figure
fig, ax = plt.subplots(figsize=(5, 3.5))
ax: mplaxes.Axes
# Pick selected rw
rws = [1.25, 1.28, 1.33]
colors = [
    plt.colormaps.get("Blues")(np.linspace(0.25, 0.75, 3)),  # type: ignore
    plt.colormaps.get("Reds")(np.linspace(0.25, 0.75, 3)),  # type: ignore
    plt.colormaps.get("Greens")(np.linspace(0.25, 0.75, 3)),  # type: ignore
]
hatches = ["---", "\\\\\\", "///"]
width = 0.2
for i, key in enumerate(files.keys()):
    print(f"------- State : {key}")
    ref = 0
    for j, rw in enumerate(rws):
        val = files[key][rw]
        val = dt.apply_systematics(val, False)
        if j == 0:
            ref = val
        val /= ref  # type: ignore
        ax.bar(
            i + (j - 1) * width,
            width=width,
            height=un.nominal_value(val),
            yerr=un.std_dev(val),
            ec=colors[i][-(j + 1)],
            fc="none",
            hatch=hatches[j],
        )
        print(f"  -> Quotient i / 0 : {un.nominal_value(val) * 100:.2f}")

ax.set_ylabel(r"$C^{2}S / C^{2}S_{1.25}$")
x = np.array(range(3))
ax.set_xticks(x)
ax.set_xticklabels(["g.s", r"$1/2^+_1$", r"$1/2^-_1$"])
ax.axhline(1, color="crimson", ls="--", lw=0.75)

# Custom legend
labels = ["1.25", "1.28", "1.33"]
patches = []
for label, hatch in zip(labels, hatches):
    patch = Patch(fc="none", ec="gray", hatch=hatch, label=label)
    patches.append(patch)
ax.legend(handles=patches, title=r"$r_{0}$ / fm", title_fontsize=16, ncols = 3)
ax.set_ylim(0, 1.75)

fig.tight_layout()
fig.savefig(sty.thesis + "systematic_rws_reduced.pdf", dpi=300)
plt.show()
