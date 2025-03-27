import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import lmfit as lm
import uncertainties as un
from scipy.interpolate import CubicSpline

import sys

sys.path.append("/media/Data/E796v2/Python/")

from parser import parse_txt
from fit import create_spline3

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


# Function to fit
def fit(exp: np.ndarray, spline: CubicSpline) -> object:
    # Create model
    def model_eval(x, sf):
        return sf * spline(x)

    # Model
    model = lm.Model(model_eval)
    res = model.fit(
        exp[:, 1],
        x=exp[:, 0],
        weights=(1.0 / exp[:, 2] ** 2) if exp.shape[1] == 3 else None,
        sf=1,
    )
    print(res.fit_report())

    ## Return fit function
    def fit_spline(x):
        return res.params["sf"].value * spline(x)

    return fit_spline, res


# ground state
gs = parse_txt("./Inputs/khan_elastic.dat")
# Theoretical model
theo_gs = parse_txt("../../Fits/pp/Inputs/g0_Khan/fort.201")
gs_spline = create_spline3(theo_gs[:, 0], theo_gs[:, 1])
gs_fit, gs_res = fit(gs, gs_spline)

# 2+ state
first = parse_txt("./Inputs/khan_inelastic.dat")
# Theoretical model
theo_first = parse_txt("../../Fits/pp/Inputs/g1_Khan/fort.202")
first_spline = create_spline3(theo_first[:, 0], theo_first[:, 1])
first_fit, first_res = fit(first, first_spline)


fig, ax = plt.subplots(1, 1, figsize=(7, 5))
ax: plt.Axes
img = plt.imshow(thesis)
# X axis
x_axis = np.linspace(10, 50, 200)
x_axis_trans = x_trans(x_axis)
# Ground-state
gs_y = gs_fit(x_axis)
ax.plot(x_axis_trans, y_trans(gs_y), lw=2, label="SF = {}".format(gs_res.uvars["sf"].format("%.2uS")))
# First
first_y = first_fit(x_axis)
ax.plot(x_axis_trans, y_trans(first_y), lw=2, label="SF = {}".format(first_res.uvars["sf"].format("%.2uS")))
ax.legend()
ax.set_axis_off()
# xsp = np.linspace(10, 50, 200)
# ysp = gs_spline(xsp)
# plt.plot(xsp, ysp, marker="")
# plt.scatter(gs[:,0], gs[:,1])

fig.tight_layout()
fig.savefig("./Outputs/khan_reanalysis.png")
plt.show()
