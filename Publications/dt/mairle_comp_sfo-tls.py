from collections import defaultdict
from re import A
from typing import Dict, List, Tuple

from scipy.fft import dst
import pyphysics as phys
import pickle
import numpy as np
import pandas as pd
import uncertainties as un
import uncertainties.unumpy as unp
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty

files = [
    "../../Fits/dt/Mairle_18Odt/Outputs/mairle.pkl",
    "../../Fits/dt/Mairle_18Odt/Outputs/reanalysis.pkl",
]
exps = []
for file in files:
    with open(file, "rb") as f:
        dic = pickle.load(f)
        exp = phys.ShellModel()
        exp.data = dic
        exps.append(exp)


# Parse theoretical data from FIGURES!
files = [
    "../../Fits/dt/Mairle_18Odt/Inputs/SFO-tls/mod1.csv",
    "../../Fits/dt/Mairle_18Odt/Inputs/SFO-tls/mod2.csv",
]
dfs = []
for file in files:
    df = pd.read_csv(file)
    top = df.columns.to_series().replace(r"^Unnamed.*", pd.NA, regex=True).ffill()
    sub = df.iloc[0]
    df.columns = pd.MultiIndex.from_arrays([top, sub])
    df = df.drop(0).reset_index(drop=True)
    dfs.append(df)

# Build ShellModels
equiv = {"d52": dt.qd52, "p12": dt.qp12, "p32": dt.qp32, "s12": dt.qs12}
theos: List[phys.ShellModel] = []
names = ["mod1", "mod2"]
for i, df in enumerate(dfs):
    theo = phys.ShellModel()
    for key, q in equiv.items():
        theo.data[q] = [
            phys.ShellModelData(float(ex), float(sf))
            for (ex, sf) in df[key].dropna().values
        ]
    # Write it to disk to avoid reading it multiple times
    with open(f"../../Fits/dt/Mairle_18Odt/Inputs/SFO-tls/{names[i]}.pkl", "wb") as f:
        pickle.dump(theo, f)
    theos.append(theo)

models = exps + theos
labels = ["Paper", "Reana", "Mod1", "Mod2"]

fig, ax = plt.subplots()

# Plot
dt.plot_bars([model.data for model in models], labels, ax, height=0.15)

fig.tight_layout()
plt.show()
