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


latex = df[["ex", "Jpi", "sf"]].map(fmt).to_latex(index=False)
print(latex)

# Mod1 SFO-tls
sfo1 = dt.build_theos(gated=True, c2s_thresh=0.07)[1]  # threshold as in vertical fig
lis = []
for q, vals in sfo1.data.items():
    for val in vals:
        lis.append((val.Ex, rf"${q.get_j_fraction()}^{{{parity(q)}}}$", val.SF))
lis = sorted(lis, key=lambda x: x[0])
contents = {
    "ex": [tup[0] for tup in lis],
    "jpi": [tup[1] for tup in lis],
    "c2s": [tup[2] for tup in lis],
}
dfsfo = pd.DataFrame(contents).map(fmt)
print(dfsfo.to_latex(index=False))


#################### Centroid/strength table
# gates = ""
gates = "_nogates"
with open(f"./Inputs/strength_centroids{gates}.pkl", "rb") as f:
    stes, cents = pickle.load(f)
with open(f"./Inputs/espes_gaps{gates}.pkl", "rb") as f:
    espes, gaps = pickle.load(f)

# Build df
qs = [dt.qd52, dt.qs12, dt.qp12, dt.qp32]
top = [q.format_simple() for q in qs]
bottom = ["Exp", "0", "1", "2"]
cols = pd.MultiIndex.from_product([top, bottom], names=["q", "m"])
idxs = ["Ste", "Cent", "ESPE"]

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
# ESPEs
lespes = []
for q in qs:
    for i in range(len(bottom)):
        lespes.append(espes[i][q])
contents.append(lespes)

df2 = pd.DataFrame(contents, columns=cols, index=idxs).map(fmt)
result = df2.loc[:, df2.columns.get_level_values(1).isin(["Exp", "0", "1"])]
print(("=") * 20, "Centroid ESPE with ", gates, ("=") * 20)
print(result.to_latex())

####################### Gaps with Baranger's formula
with open("./Inputs/espes_gaps_nogates.pkl", "rb") as f:
    _, nogaps = pickle.load(f)
top = ["6", "8", "14"]
bottom = ["yes", "no"]
cols = pd.MultiIndex.from_product([top, bottom], names=["gap", "cond"])
idxs = ["Exp", "SFO-tls", "Mod1", "Mod2"]
contents = []
for i, model in enumerate(gaps):
    aux = []
    for gated, notgated in zip(model[::-1], nogaps[i][::-1]):
        aux.append(gated)
        aux.append(notgated)
    contents.append(aux)
df3 = pd.DataFrame(contents, columns=cols, index=idxs).map(fmt)
print(("=") * 20, "Gaps with ESPEs", ("=") * 10)
print(df3.to_latex())

############################## N=6 evolution
with open("./Inputs/evolution.pkl", "rb") as f:
    stes, cents, gaps = pickle.load(f)
top = ["16O", "18O", "20O"]
bottom = ["Exp", "SFO-tls", "Mod1"]
cols = pd.MultiIndex.from_product([top, bottom], names=["nucleus", "model"])
idxs = ["ste 0p1/2", "cent 0p1/2", "ste 0p3/2", "cent 0p3/2", "gap"]
contents = []
# 0p1/2
for lis in [stes, cents]:
    aux = []
    for val in lis:
        aux.append(val[dt.qp12])
    contents.append(aux)
# 0p3/2
for lis in [stes, cents]:
    aux = []
    for val in lis:
        aux.append(val[dt.qp32])
    contents.append(aux)
# Gaps
contents.append(gaps)
df4 = pd.DataFrame(contents, columns=cols, index=idxs).map(fmt)
print(("=") * 20, "Gaps for 16-20O", ("=") * 10)
print(df4.to_latex())
