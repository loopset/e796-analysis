import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import lmfit as lm
import uncertainties as un
from scipy.interpolate import CubicSpline

import sys

sys.path.append("/media/Data/E796v2/Python/")
plt.style.use("../../Python/actroot.mplstyle")

import pyphysics as phys

# Read the image
thesis = plt.imread("./Inputs/khan_thesis.png")


def convert():
    x_pixels = [104.2, 221.92]
    x_axis = [10.0, 30.0]
    y_pixels = [230.98, 170.54]
    y_axis = [10.0, 100.0]

    # Transform x axis
    x_trans = np.poly1d(np.polyfit(x_axis, x_pixels, 1))
    y_log_axis = np.log10(y_axis)
    # y_log_axis = y_axis
    y_trans = np.poly1d(np.polyfit(y_log_axis, y_pixels, 1))

    return x_trans, lambda y: y_trans(np.log10(y))


# Define transformation functions
x_trans, y_trans = convert()


# Ground-state
gs = phys.parse_txt("./Inputs/khan_elastic.dat")
gs_comp = phys.Comparator(gs)
gs_comp.add_model("BG", "../../Fits/pp/Inputs/g0_Khan/fort.201")
gs_comp.fit()

# 2+ state
first = phys.parse_txt("./Inputs/khan_inelastic.dat")
first_comp = phys.Comparator(first)
first_comp.add_model("BG", "../../Fits/pp/Inputs/g1_Khan/fort.202")
first_comp.fit()

# 3+ state
sec = phys.parse_txt("./Inputs/khan_3minus.dat")
sec_comp = phys.Comparator(sec)
sec_comp.add_model("BG", "../../Fits/pp/Inputs/g3_Khan/fort.202")
sec_comp.fit()

fig, ax = plt.subplots(1, 1, figsize=(7, 5))
ax: plt.Axes
img = plt.imshow(thesis)
# X axis
x_axis = np.linspace(8, 50, 200)
x_axis_trans = x_trans(x_axis)
# Ground-state
gs_y = gs_comp.eval(x_axis)
ax.plot(
    x_axis_trans,
    y_trans(gs_y),
    lw=2,
    label=r"Exp = BG $\times$ {}".format(gs_comp.get_sf().format("%.2uS")),
)
# First
first_y = first_comp.eval(x_axis)
ax.plot(
    x_axis_trans,
    y_trans(first_y),
    lw=2,
    label=r"$\beta_2 = 0.581$",
)
# Second
sec_y = sec_comp.eval(x_axis) * 0.1
ax.plot(
    x_axis_trans,
    y_trans(sec_y),
    lw=2,
    label=r"$\beta_{3} = 0.331$",
)
ax.legend(loc="upper center", fontsize=12)
ax.set_axis_off()
# xsp = np.linspace(10, 50, 200)
# ysp = gs_spline(xsp)
# plt.plot(xsp, ysp, marker="")
# plt.scatter(gs[:,0], gs[:,1])

fig.tight_layout()
fig.savefig("./Outputs/khan_reanalysis.png")
plt.show()
