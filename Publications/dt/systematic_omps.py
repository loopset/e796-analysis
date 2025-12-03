import pyphysics as phys
from pyphysics.actroot_interface import SFInterface
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import pandas as pd

import sys

sys.path.append("../")
import styling as sty

sfs = SFInterface("../../Fits/dt/Macros/Outputs/systematic_omps.root")
pairs = [
    tuple(part.strip() for part in model.fName.split("+")) for model in sfs.get("g0")
]
# Create empty df
df = pd.DataFrame(
    0,
    index=list(set([l for l, _ in pairs[::-1]])),
    columns=list(set([r for _, r in pairs[::-1]])),
    dtype=object
)

# Fill it
for model in sfs.get("g0"):
    a, b = model.fName.split("+")
    # a, b = a.split(), b.split()
    a = a.strip()
    b = b.strip()
    df.at[a, b] = model.fSF #type: ignore

print(df.map(lambda x: f"{x:.2uS}").to_latex())

# Compute sys uncertainty
ref = df.at["Daeh", "Pang"]
sigma = np.std(unp.nominal_values(df.values - ref)) 
rel = (un.nominal_value(ref) + sigma) / un.nominal_value(ref) * 100 - 100 #type: ignore
print("OMP systematic unc: ", rel, " %")


fig, ax = plt.subplots()
ax: mplaxes.Axes
sfs.plot_exp("g0", ax=ax)
sfs.plot_models("g0", ax=ax)
sfs.format_ax("g0", ax)
ax.legend()

plt.show()
