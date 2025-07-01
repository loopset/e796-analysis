from cProfile import label
import pickle
from matplotlib import hatch
import uncertainties as un
from typing import Dict, Union, List
import pyphysics as phys
import matplotlib.pyplot as plt

png = plt.imread("./Inputs/mairle_dt_centroids.png")

# Transform
x1, x2 = (140, 15), (407, 5)
y1, y2 = (221, 0), (91, 2)
transx, transy = phys.utils.create_trans_imshow(x1, x2, y1, y2)

# Read data
with open("./Outputs/mairle.pkl", "rb") as f:
    mairle = pickle.load(f)
with open("./Outputs/reanalysis.pkl", "rb") as f:
    reana = pickle.load(f)

RetType = Dict[phys.QuantumNumbers, List[Union[float, un.UFloat]]]

qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)


def centroids(d: phys.SMDataDict) -> RetType:
    ret: RetType = {}
    for q, vals in d.items():
        if q not in ret:
            ret[q] = [0, 0]
        ste = sum(val.SF for val in vals)  # type: ignore
        ret[q][1] = ste
        for val in vals:
            ret[q][0] += val.SF * val.Ex / ste  # type: ignore
    return ret


styles = {
    qp12: {"marker": "h", "color": "green", "hatch": r"--"},
    qp32: {"marker": "h", "color": "grey", "hatch": "//"},
}


fig, ax = plt.subplots()
ax.imshow(png)

# Plot ours
for i, data in enumerate([mairle, reana]):
    cents = centroids(data)
    for q in [qp12, qp32]:
        if not q in data:
            continue
        ex, ste = cents[q]
        ex_trans = transx(un.nominal_value(ex))
        ste_trans = transy(un.nominal_value(ste))
        zero = transy(0)
        height = abs(ste_trans - zero)
        ax.bar(
            ex_trans,
            height=-height,
            bottom=transy(0),
            width=8,
            color="none",
            ec=styles[q]["color"],
            hatch=styles[q]["hatch"],
            lw=1.5,
            alpha=0.5 if i == 0 else 1,
            label=None if i == 0 else q.format(),
        )
ax.legend(loc="upper right", frameon=True, framealpha=1, title="T = 1/2", fontsize=10)
ax.set_axis_off()

fig.tight_layout()
fig.savefig("./Outputs/centroids.png")

plt.show()
