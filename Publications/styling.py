import cycler
import pyphysics

base2d = {
    "flow": "none",
    "cmin": 1,
    "cmap": "managua_r",
    "rasterized": True,
    "cbarpad": 0.05,
    "cbarsize": 0.2,
    "cbarextend": False,
}

errorbar = {"ls": "none", "marker": "s", "capsize": 3}

base1d = {
    "histtype": "step",
    "yerr": False,
    "flow": "none",
    # "capsize": 0,
    "lw": 1,
    # "marker": "none",
}

styles = {
    "ex": {
        "histtype": "errorbar",
        "yerr": True,
        "flow": "none",
        "xerr": True,
        "capsize": 0,
        "color": "black",
        "lw": 1,
        "marker": "none",
    },
    "global": {
        "lw": 1.5,
        "color": "red",
    },
    "ps": {
        "histtype": "step",
        "yerr": False,
        "flow": "none",
        "lw": 1,
    },
    "sn": {
        "lw": 2,
        "ls": "dotted",
    },
}

# colors = {"l0": "#57d1c9", "l1": "#ea5b67", "l2": "#555d89"} # from Juan's thesis
colors = {
    "l0": "#a01525",
    "l1": "#28a83c",
    "l2": "#5a99d8",
}  # from Juan's 15C cross-shell states... paper
ls = {"l0": "--", "l1": "-", "l2": ":"}

cyclers = {
    "l012": cycler.cycler(color=[colors["l0"], colors["l1"], colors["l2"]])
    + cycler.cycler(ls=[ls["l0"], ls["l1"], ls["l2"]]),
    "l12": cycler.cycler(color=[colors["l1"], colors["l2"]])
    + cycler.cycler(ls=[ls["l1"], ls["l2"]]),
}

barplot = {
    pyphysics.QuantumNumbers.from_str("0p1/2"): dict(
        fc="none", ec="green", hatch=r"--"
    ),
    pyphysics.QuantumNumbers.from_str("0p3/2"): dict(
        fc="none", ec="dimgray", hatch="//"
    ),
    pyphysics.QuantumNumbers.from_str("0d5/2"): dict(
        fc="none", ec="dodgerblue", hatch=".."
    ),
    pyphysics.QuantumNumbers.from_str("0d3/2"): dict(
        fc="none", ec="lightskyblue", hatch="oo"
    ),
    pyphysics.QuantumNumbers.from_str("1s1/2"): dict(
        fc="none", ec="crimson", hatch=r"\\"
    ),
}

# Path of thesis
thesis = "/media/Data/Thesis/figures/analysis_e796/"