from typing import Dict, List
import pyphysics as phys
import uncertainties as un
import pandas as pd
import matplotlib.pyplot as plt

import ROOT as r

file = r.TFile("./Inputs/iter_v567.root")  # type: ignore

states = ["v5", "v6", "v7"]
vals: Dict[str, List[un.Variable]] = {}
for state in states:
    vals[state] = []
    for l in [0, 1, 2]:
        key = f"hSF{state}{l}"
        h = file.Get(key)
        mean = h.GetMean()
        sigma = h.GetStdDev()
        val = un.ufloat(mean, sigma)
        vals[state].append(val)
df = pd.DataFrame(vals)
latex = df.map(lambda x: f"{x:.2uS}").to_latex()
print(latex)
