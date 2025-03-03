import sys

sys.path.append("/media/Data/E796v2/Python/")

import uncertainties as unc
import ROOT as r

import shell_model as sm
from interfaces import FitInterface
from physics import Barager

path = "/media/Data/E796v2/Fits/dt/"

# Experimental dt data
inter = FitInterface(
    path + "Outputs/interface.root",
    path + "Outputs/fit_juan_RPx.root",
    path + "Outputs/sfs.root",
)

# Declare quantumm numbers
q52 = sm.QuantumNumbers(0, 2, 2.5)
q12 = sm.QuantumNumbers(0, 1, 1.5)

# Removal reactions
rem = inter.map(
    {
        q52: ["g0"],
        q12: ["g2", "v0", "v1", "v4"],
    }
)

# Adding reactions: B. Fernández-Domínguez PRC 84 (2011)
add = sm.ShellModel()
add.data = {q52: [sm.ShellModelData(unc.ufloat(0, 0), unc.ufloat(0.34, 0.03))], q12: []}

# YSOX
ysox = sm.ShellModel(
    (
        [
            path + "Inputs/SM/log_O20_O19_ysox_tr_j0p_m1p.txt",
            path + "Inputs/SM/log_O20_O19_ysox_tr_j0p_m1p.txt",
        ]
    )
)

# Binding energies
snadd = r.ActPhysics.Particle("21O").GetSn()
snrem = r.ActPhysics.Particle("20O").GetSn()

# Experimental ESPE
expe = Barager()
expe.set_removal(rem, snrem)
expe.set_adding(add, snadd)
expe.do_for([q52, q12])

for key, val in expe.Results.items():
    print(val)

expgap = expe.get_gap(q52, q12)
print("Experimental gap: ", expgap)