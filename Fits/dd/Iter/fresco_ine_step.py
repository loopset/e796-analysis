import math
import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes

# Do the reduced matrix calculation
# 2: I. Zanon
# 3: theoretical result
labels = ["2+1", "3+1", "2+2"]
ls = [2, 3, 2]
em = [5.9, 1.6e3, 1.3]
isUp = [False, True, False]
# Particle
p = phys.Particle(8, 20)

# Daehnick radii
radii = {"RealVol": 1.17, "ImVol": 1.325, "ImSurf": 1.325}

# But according to Fresco's manual, the RDEF(k) = DEF(k) for I = K = 0. (Clebsch-Gordan and so equal to 1)
for i, (label, exp) in enumerate(zip(labels, em)):
    beta = phys.BE_to_beta(exp, p, ls[i], isUp[i])
    print(f"-> {label}")
    # 0 -> L B(EL)
    coulomb = exp
    if not isUp[i]:
        coulomb *= 2 * ls[i] + 1
    print(f"   Coulomb : {math.sqrt(coulomb):.3f}")
    for comp, r in radii.items():
        delta = beta * r * math.pow(p.fA, 1.0 / 3)  # type: ignore
        print(f"   {comp} : {delta:.3f}")
