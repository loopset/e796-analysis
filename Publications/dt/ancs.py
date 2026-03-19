import pickle
import pyphysics as phys

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mpcolor
import matplotlib.lines as mplines
import uncertainties as un
import uncertainties.umath as umath
import uncertainties.unumpy as unp
import pandas as pd

import sys

sys.path.append("../")
import styling as sty
import dt

################## Ex and C2S table

df = dt.build_df()[["ex", "sf", "model"]]


def parity(q: phys.QuantumNumbers):
    if q.l == 0 or q.l == 2:
        return "+"
    else:
        return "-"


df["Jpi"] = [
    rf"${q.get_j_fraction()}^{{{parity(q)}}}$"
    for key, (sf, q) in dt.assignments.items()
]


def fmt(val):
    if hasattr(val, "nominal_value"):
        return f"{val:.2uS}"
    elif isinstance(val, float):
        return f"{val:.2f}"
    else:
        return val


# b
bs = [
    2.1641,
    -9.1718,
    5.5483,
    6.5763,
    8.1480,
    9.2488,
    10.1129,
    11.8419,
    13.5893,
    15.2579,
    16.5644,
]

df["b"] = bs
df["C"] = df["b"] * unp.sqrt(df["sf"]) #type: ignore

print(df[["ex", "Jpi", "sf", "b", "C"]].map(fmt).to_latex())
