import pyphysics as phys
from pyphysics.actroot_interface import FitInterface, SFInterface
import hist
import uproot

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

pp = SFInterface("../../Fits/pp/Outputs/sfs.root")
dd = SFInterface("../../Fits/dd/Outputs/sfs.root")
sfs = [pp, dd]

state = "g0"
dfs = []
for sf in sfs:
    df = pd.DataFrame(
        {str(model.fName): [model.fSF, model.fIntSF] for model in sf.fSFs[state][::-1]},
        index=[0, 1],
    )
    dfs.append(df)
stack = pd.concat(dfs, axis=1, keys=["pp", "dd"])
stack.columns = pd.MultiIndex.from_tuples(
    [(f"{{{lvl0}}}", f"{{{lvl1}}}") for lvl0, lvl1 in stack.columns]
)
latex = stack.map(lambda x: f"{x:2uS}").to_latex(float_format="{:2uS}".format)
print(latex)
