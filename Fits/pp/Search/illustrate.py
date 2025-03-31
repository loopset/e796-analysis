import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import os

plt.style.use("../../../Python/actroot.mplstyle")

def parse_columns(file: str, ncols: int = 2) -> np.ndarray:
    with open(file) as f:
        lines = f.readlines()
    ret = []
    for line in lines:
        split = line.split()
        if len(split) == ncols:
            try:
                ret.append([float(col) for col in split])
            except: ## probably we are parsing a string; skip in this case
                continue
    return np.array(ret)
    

maindir = "./g1_BG/"

models = {}
for dir in os.listdir(maindir):
    fresco = f"{maindir}/{dir}/fort.202"
    if os.path.exists(fresco):
        it = dir.find("_")
        beta = dir[it + 1:]
        models[float(beta)] = parse_columns(fresco)

exp = parse_columns("../Outputs/xs/g1_xs.dat", 3)

# Draw
fig, ax = plt.subplots(1, 1, figsize=(6,4))
ax: plt.Axes
for i, (key, vals) in enumerate(models.items()):
    ax.plot(vals[:,0], vals[:,1], marker="", lw=1.5)
# ax.errorbar(exp[:,0], exp[:,1], yerr=exp[:,2], ls="None", lw=1.5, marker="s", mew=1.5)
ax.annotate("", xy=(15, 32), xytext=(15, 0.5), arrowprops=dict(arrowstyle="->"))
ax.annotate(r"$\beta_{2} \uparrow$", xy=(18, 27), fontsize=16, ha="center", va="center")
# Axis settings
ax.set_xlim(5, 50)
ax.set_xlabel(r"$\theta_{\mathrm{CM}} [^{\circ}]$")
ax.set_ylabel(r"$\mathrm{d}\sigma / \mathrm{d}\Omega [\mathrm{mb} / \mathrm{sr}]$")

fig.tight_layout()
plt.savefig("./Pictures/beta2_illustration.png", dpi=200)
plt.show()

# model = parse_two_column("./g1_BG/beta_0.200/fort.202")
