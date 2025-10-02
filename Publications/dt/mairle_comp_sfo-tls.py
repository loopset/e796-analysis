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

files = ["../../Fits/dt/Mairle_18Odt/Inputs/SFO-tls/mod1.csv", "../../Fits/dt/Mairle_18Odt/Inputs/SFO-tls/mod2.csv"]
dfs = []
for file in files:
    df = pd.read_csv(file)
    top = df.columns.to_series().replace(r"^Unnamed.*", pd.NA, regex=True).ffill()
    sub = df.iloc[0]
    df.columns = pd.MultiIndex.from_arrays([top, sub])
    df = df.drop(0).reset_index(drop=True)
    dfs.append(df)