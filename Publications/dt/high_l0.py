import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as un

sfo = phys.ShellModel(
    [
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        "../../Fits/dt/Inputs/SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)
sfo.set_min_SF(0.005)

qs = [phys.QuantumNumbers(0, 1, 0.5), phys.QuantumNumbers(0, 1, 1.5)]

fig, ax = plt.subplots(1, 1)

width = 0.25
for i, q in enumerate(qs):
    vals = sfo.data.get(q)
    if vals is None:
        continue
    for val in vals:
        x = i + 0
        y = un.nominal_value(val.Ex)
        ax.plot([x - width / 2, x + width / 2], [y] * 2, lw=2)
        ax.annotate(f"{val.SF:.2f}", xy=(x  + width / 2 + 0.05, y), ha="center", va="center", fontsize=8)

ax.set_ylabel(r"E$_{x}$ [MeV]")
ax.set_xlim(0 - width , len(qs) + width)
ax.set_xticks(range(len(qs)), [fr"{q.format()}" for q in qs])
fig.tight_layout()
plt.show()
