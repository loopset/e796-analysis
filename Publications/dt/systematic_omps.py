import pyphysics as phys
from pyphysics.actroot_interface import SFInterface
import numpy as np
import uncertainties as un
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import pandas as pd

import sys

sys.path.append("../")
import styling as sty

sfs = SFInterface("../../Fits/dt/Macros/Outputs/systematic_omps.root")

df = pd.DataFrame(
    {
        "Model": [str(sf.fName) for sf in sfs.fSFs["g0"][::-1]],
        "SF": [sf.fSF for sf in sfs.fSFs["g0"][::-1]],
        "SF.n": [sf.fSF.n for sf in sfs.fSFs["g0"][::-1]],
    }
)

## Compute sistematic uncertainty
ref = df["SF.n"].iloc[0]
## Incoming OMP
inc = df.iloc[:3]
inc_std = (inc["SF.n"] - ref).std()
inc_rel = (ref + inc_std) / ref * 100 - 100
print("Incoming OMP sys unc: ", inc_rel)

## Outgoing OMP
out = df.iloc[[0, -1]]
out_std = (out["SF.n"] - ref).std()
out_rel = (ref + out_std) / ref * 100 - 100
print("Outgoing OMP sys unc: ", out_rel)


fig, ax = plt.subplots()
ax: mplaxes.Axes
sfs.plot_exp("g0", ax=ax)
sfs.plot_models("g0", ax=ax)
sfs.format_ax("g0", ax)
ax.legend()

plt.show()
