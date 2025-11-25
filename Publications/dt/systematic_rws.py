from collections import defaultdict
import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as un
from pathlib import Path
import re

import sys

sys.path.append("../")
import styling as sty

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
plt.show()
