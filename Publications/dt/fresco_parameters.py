from pyphysics.actroot_interface import FitInterface
from pyphysics.actroot_interface import KinInterface
import pyphysics as phys
import uncertainties as un
import pandas as pd
import ROOT as r

dt = FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")

# Substract ground-state energy offet
gs = "g0"
offset = dt.get(gs)[0]
# 20O particle
part = phys.Particle("20O")


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


def format_pang(ebeam):
    omp = r.PhysOMP.Pang(8, 19, ebeam, True)  # type: ignore
    pang = f"""
 &pot kp=2 type=0 p(1:7)= {omp.fA:.4f}  0.0000  {omp.frc:.4f}  0.000  0.0000  0.0000  0.0000 /
 &pot kp=2 type=1 p(1:7)= {omp.fVr:.4f}  {omp.frv:.4f}  {omp.fav:.4f}  {omp.fWv:.4f}  {omp.frw:.4f}  {omp.faw:.4f}  0.0000 /
 &pot kp=2 type=2 p(1:7)= 0.0000  0.0000   0.0000   {omp.fWs:.4f}  {omp.frs:.4f}  {omp.fas:.4f}  0.0000 /
"""
    print(pang)


def call_for(state: str) -> None:
    row = df.loc[df["state"] == state].iloc[0]
    ex = float(row["Ex"])
    equiv = float(row["EquivBeam"])
    sn = float(row["Sn"])
    print(f"State     : {state}")
    print(f"Ex        : {ex}")
    print(f"EquivBeam : {equiv}")
    print(f"Sn        : {sn}")
    print(f"Pang OMP  : ")
    format_pang(equiv)
