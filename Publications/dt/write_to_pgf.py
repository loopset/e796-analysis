import pyphysics as phys
import pickle
import sys
import uncertainties as un

sys.path.append("./")
sys.path.append("../")
import dt
import styling as sty

## Experimental dataset
exp = dt.build_sm()

# Unmodified SFO-tls
sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
    ]
)
sfo.set_max_Ex(16.5)
sfo.set_min_SF(0.04)

lst = sorted(
    [(o.Ex, o.SF, q) for q, lis in sfo.data.items() for o in lis],
    key=lambda x: un.nominal_value(x[0]),
)
with open("./Outputs/sfo_centroid_example.txt", "w") as f:
    f.write(f"ex sf meta\n")
    for ex, sf, q in lst:
        q: phys.QuantumNumbers
        aux = f"{q.l}{int(q.j)}"
        f.write(f"{ex} {sf} {aux}\n")

# Compute centroids
cents = dt.get_centroids(sfo.data)
print(cents)
