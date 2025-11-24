from pyphysics.actroot_interface import FitInterface
from pyphysics.actroot_interface import KinInterface
import pyphysics as phys
import uncertainties as un
import pandas as pd

dt = FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")

# Substract ground-state energy offet
gs = "g0"
offset = dt.get(gs)[0]
# 19O particle
part = phys.Particle("19O")

rows = []
for key in dt.fAmps.keys():
    ex = dt.get(key)[0]
    ex_corr = ex - offset  # type: ignore
    # Equivalent beam energy
    kin = KinInterface(f"d(20O,19O)@70|{un.nominal_value(ex_corr)}")
    equiv = kin.fKin.ComputeEquivalentBeamEnergy()
    # Sn
    sn = part.get_sn() + ex_corr

    rows.append(
        {
            "state": key,
            "Ex": f"{un.nominal_value(ex_corr):.4f}",
            "EquivBeam": f"{equiv: .4f}",
            "Sn": f"{un.nominal_value(sn):.4f}",
        }
    )


df = pd.DataFrame(rows)[["state", "Ex", "EquivBeam", "Sn"]]
print(df.to_string(index=False))
