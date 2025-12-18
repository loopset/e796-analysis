import pickle
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

################## Ex and C2S table

df = dt.build_df()[["ex", "sf", "model"]]


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
    elif isinstance(val, float):
        return f"{val:.2f}"
    else:
        return val


latex = df[["ex", "Jpi", "orbital", "sf"]].map(fmt).to_latex(index=False)
print(latex)

#################### Centroid/strength table
with open("./Inputs/strength_centroids.pkl", "rb") as f:
    stes, cents = pickle.load(f)

# Build df
qs = [dt.qd52, dt.qs12, dt.qp12, dt.qp32]
top = [q.format_simple() for q in qs]
bottom = ["Exp", "0", "1", "2"]
cols = pd.MultiIndex.from_product([top, bottom], names=["q", "m"])
idxs = ["Ste", "Cent"]

contents = []
# Strengths
lste = []
for q in qs:
    for i in range(len(bottom)):
        lste.append(stes[i][q])
contents.append(lste)
# Centroids
lcent = []
for q in qs:
    for i in range(len(bottom)):
        lcent.append(cents[i][q])
contents.append(lcent)

df2 = pd.DataFrame(contents, columns=cols, index=idxs)
df2 = df2.map(fmt)
print(df2.to_latex())
