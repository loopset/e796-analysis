import cycler

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

colors = {"l0": "#57d1c9", "l1": "#ea5b67", "l2": "#555d89"}

cyclers = {"l012": cycler.cycler(color=[colors["l0"], colors["l1"], colors["l2"]])}
