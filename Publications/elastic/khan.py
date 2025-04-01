import matplotlib as mpl
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt
import numpy as np
import lmfit as lm
import uncertainties as un
import uncertainties.unumpy as unp
from scipy.interpolate import CubicSpline

import sys
import os

sys.path.append("/media/Data/E796v2/Python/")
plt.style.use("../../Python/actroot.mplstyle")

import pyphysics as phys

# Read the image
thesis = plt.imread("./Inputs/khan_thesis.png")

# Define transformation functions
x_trans, y_trans = phys.create_trans_imshow(
    (104.2, 10), (221.92, 30), (230.98, 10), (170.54, 100), False, True
)

# Experimental data
exp = {
    "gs": "./Inputs/khan_elastic.dat",
    "first": "./Inputs/khan_inelastic.dat",
    "second": "./Inputs/khan_3minus.dat",
}

# Theoretical lines
theo = {
    "gs": "../../Fits/pp/Inputs/g0_Khan/fort.201",
    "first": "../../Fits/pp/Inputs/g1_Khan/fort.202",
    "second": "../../Fits/pp/Inputs/g3_Khan/fort.202",
}

## Fit full range data
comps = {}
for key, ex in exp.items():
    data = phys.parse_txt(ex)
    c = phys.Comparator(data)
    c.add_model("BG", theo[key])
    c.fit()
    comps[key] = c


## Limited range data
def apply_cm_gate(data: np.ndarray) -> np.ndarray:
    mask = np.logical_and(data[:, 0] >= 18, data[:, 0] <= 25)
    return data[mask, :]


gated = {}
for key, ex in exp.items():
    data = phys.parse_txt(ex)
    data = apply_cm_gate(data)
    c = phys.Comparator(data)
    c.add_model("BG", theo[key])
    c.fit()
    gated[key] = c

# ## Search for beta
# root = {
#     "first": "../../Fits/pp/Search/Outputs/inter_g1_Khan.dat",
#     "second": "../../Fits/pp/Search/Outputs/inter_g3_Khan.dat",
# }

# dirs = {
#     "first": "../../Fits/pp/Search/g1_Khan/",
#     "second": "../../Fits/pp/Search/g1_Khan/",
# }

# search = {}

# for key, path in dirs.items():
#     search[key] = {}
#     for dir in os.listdir(path):
#         fresco = f"{path}/{dir}/fort.202"
#         if os.path.exists(fresco):
#             it = dir.find("_")
#             beta = dir[it + 1 :]
#             search[key][float(beta)] = fresco


# def do_search(exp: np.ndarray, models: dict) -> float:
#     c = phys.Comparator(exp)
#     for beta, file in models.items():
#         c.add_model(str(beta), file)
#     c.fit()
#     sfs = []
#     betas = []
#     for name, sf in c.fSFs.items():
#         betas.append(float(name))
#         sfs.append(sf)
#     sfs = np.array(sfs)
#     betas = np.array(betas)
#     # And now search
#     interp = phys.create_interp1d(betas, unp.nominal_values(sfs))
#     root = phys.find_root(interp, 1, [betas.min(), betas.max()])
#     return root

# # Test
# first = phys.parse_txt("./Inputs/khan_inelastic.dat")
# do_search(first, search["first"])

# Determined from ROOT program
beta_all = {"first": 0.58, "second": 0.344}
beta_gated = {"first": 0.68, "second": 0.24}
# for key, file in root.items():
#     data = phys.parse_txt(file, 3)
#     spe = phys.create_interp1d(data[:, 0], data[:, 1])
#     for i, dictionary in enumerate([comps, gated]):
#         c = dictionary[key]
#         beta = phys.find_root(
#             spe, 1 / c.get_sf().n, [data[:, 0].min(), data[:, 0].max()]
#         )
#         print(f"Inversed : {1 / c.get_sf()}")
#         if i == 0:
#             beta_all[key] = beta
#         else:
#             beta_gated[key] = beta


fig, ax = plt.subplots(1, 1, figsize=(7, 5))
ax: mplaxes.Axes
img = plt.imshow(thesis)
colors = plt.cm.tab10.colors[:3]

# Full range
x_axis = np.linspace(8, 50, 200)
x_axis_trans = x_trans(x_axis)
for i, (key, c) in enumerate(comps.items()):
    y = c.eval(x_axis)
    if key == "second":
        y *= 0.1
    # Label
    if key == "gs":
        label = f"{c.get_sf():.2uS}"
    else:
        label = rf"$\beta_{{{i + 1}}} = $ {beta_all[key]:.2f}"
    ax.plot(x_axis_trans, y_trans(y), lw=2, label=label, color=colors[i])

# Limited range
x_gated = np.linspace(18, 25, 50)
x_gated_trans = x_trans(x_gated)
for i, (key, c) in enumerate(gated.items()):
    y = c.eval(x_gated)
    if key == "second":
        y *= 0.1
    # Label
    if key == "gs":
        label = f"{c.get_sf():.2uS}"
    else:
        label = rf"$\beta_{{{i+1}}} = $ {beta_gated[key]:.2f}"
    ax.plot(x_gated_trans, y_trans(y), ls="dashed", lw=2, label=label, color=colors[i])
ax.legend(loc="upper center", fontsize=12, ncols=2)
# vlines
ax.axvline(x_trans(18), ymin=0.15, ymax=0.85, color="crimson", lw=2, ls="--")
ax.axvline(x_trans(25), ymin=0.15, ymax=0.85, color="crimson", lw=2, ls="--")
ax.set_axis_off()

fig.tight_layout()
fig.savefig("./Outputs/khan_reanalysis.png", dpi=200)
plt.show()
