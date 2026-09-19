import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt
import uproot

# which efficiency
which = "eff2"

# Large lateral Si -> old front Si
large = uproot.open("../Outputs/17C_d_t_large_si.root")
eff_large = large[which]

# Small lateral Si -> new front Si (S2384)
small = uproot.open("../Outputs/17C_d_t_small_si.root")
eff_small = small[which]

effs = [eff_small, eff_large]
labels = ["New 1.5 um Si", "Old 0.5 um Si"]

plt.rcParams["axes.labelsize"] = 16
fig, ax = plt.subplots(figsize=(4, 2.75), constrained_layout=True)
for i, eff in enumerate(effs):
    x = eff.values(0) #type: ignore
    y = eff.values(1) #type: ignore
    n = 2
    x = np.append(x[:len(x)-len(x)%n].reshape(-1, n).mean(axis=1), x[len(x)-len(x)%n:].mean() if len(x)%n else [])
    y = np.append(y[:len(y)-len(y)%n].reshape(-1, n).mean(axis=1), y[len(y)-len(y)%n:].mean() if len(y)%n else [])
    ax.plot(x, y, label=labels[i])

# L1 region
l1 = 17
ax.axvline(l1, lw=1, ls="--", color="gray")
ax.axvspan(0, l1, color="gray", alpha=0.1)

ax.legend(fontsize=12)
ax.set_xlim(0, 80)
ax.set_ylim(0, 1)
ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
ax.set_ylabel("Efficiency")

fig.savefig("./Outputs/compEffs_dt.png", dpi=300)

plt.show()