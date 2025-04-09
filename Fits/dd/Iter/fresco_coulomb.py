import ROOT as r
import os
import re
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import pyphysics as phys

# Determine state
state = "g5"
par = "p3"

# Read experimental xs
with r.TFile("../Outputs/xs.root") as f:  # type: ignore
    gexp = f.Get(f"g{state}")

# And plot
comp = phys.Comparator(phys.parse_tgraph(gexp))

pattern = re.compile(rf"{state}_coulomb_(\d+(\.\d+)?)")

base = "./Inputs/fresco_coulombdef/"
dirs = {}
for subdir in os.listdir(base):
    path = os.path.join(base, subdir)
    if os.path.isdir(path):
        fresco = os.path.join(path, "fort.202")
        if os.path.exists(fresco):
            match = pattern.match(subdir)
            if match:
                dirs[match.group(1)] = fresco
# Sort by creation date
dirs = {
    k: v for k, v in sorted(dirs.items(), key=lambda item: os.path.getmtime(item[1]))
}
for k, v in dirs.items():
    comp.add_model(fr"$\sqrt{{B(E3)}} = {k}$", v)
comp.fit()

fig, ax = plt.subplots(1, 1, figsize=(7, 5))
ax: mplaxes.Axes
comp.draw(ax=ax)
fig.tight_layout()
fig.savefig("./Outputs/fresco_be3.png", dpi=200)
plt.show()
