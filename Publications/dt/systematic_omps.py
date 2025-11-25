import pyphysics as phys
from pyphysics.actroot_interface import SFInterface
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import pandas as pd

import sys

sys.path.append("../")
import styling as sty

sfs = SFInterface("../../Fits/dt/Macros/Outputs/systematic_omps.root")

df = pd.DataFrame(
    {
        "Model": [sf.fName for sf in sfs.fSFs["g0"][::-1]],
        "SF": [sf.fSF for sf in sfs.fSFs["g0"][::-1]],
    }
)
df["SF"] = df["SF"].map(lambda x: f"{x:.2uS}")
latex = df.T.to_latex()
print(latex)

fig, ax = plt.subplots()
ax: mplaxes.Axes
sfs.plot_exp("g0", ax=ax)
sfs.plot_models("g0", ax=ax)
sfs.format_ax("g0", ax)

plt.show()
