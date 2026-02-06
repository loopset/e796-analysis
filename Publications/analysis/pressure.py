from typing import List
import pyphysics as phys
import hist
import uproot

import numpy as np
import uncertainties as un
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick
import matplotlib.colors as mplcolor
import pandas as pd

import sys

sys.path.append("../")
import styling as sty

# Read df
df = pd.read_excel("./Inputs/pressure.ods")
df["Run"] = df["Run"] - df["Run"].min()
# Mean
mean = df["Pressure"].mean()
sigma = df["Pressure"].std()
p = un.ufloat(mean, sigma)
print(f"Mean + std = {p:.2uS}")


fig, ax = plt.subplots(figsize=(4.5, 3.5))
ax.plot(df["Run"], df["Pressure"], marker="o", ms=4, ls="-")
ax.set_ylim(930, 970)
ax.axhline(mean, color="crimson", ls="--", lw=1.5)
ax.annotate(
    rf"$\mu$ = {mean:.1f} mbar", xy=(0.25, 0.85), xycoords="axes fraction", **sty.ann
)
ax.annotate(
    rf"$\sigma$ = {sigma:.1f} mbar", xy=(0.25, 0.775), xycoords="axes fraction", **sty.ann
)

ax.set_xlabel("Run number")
ax.set_ylabel("Pressure [mbar]")

fig.tight_layout()
fig.savefig(sty.thesis + "pressure.pdf", dpi=300)
plt.show()
