import pyphysics as phys

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mpcolor
import matplotlib.lines as mplines
import uncertainties as un
import pandas as pd

import sys

sys.path.append("../")
import styling as sty
import dt

df = dt.build_df(True)[["ex", "sf", "model"]]


def parity(q: phys.QuantumNumbers):
    if q.l == 0 or q.l == 2:
        return "+"
    else:
        return "-"

df["orbital"] = [q.format() for _, (_, q) in dt.assignments.items()]

df["Jpi"] = [
    rf"${q.get_j_fraction()}^{{{parity(q)}}}$"
    for key, (sf, q) in dt.assignments.items()
]


def fmt(val):
    if hasattr(val, "nominal_value"):
        return f"{val:.2uS}"
    else:
        return val


latex = df[["ex", "Jpi", "orbital", "sf"]].map(fmt).to_latex(index=False)
print(latex)
