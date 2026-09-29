import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt

omps = ["DaehPang", "HaixiaHT1p"]

waves = {"s": 202, "p": 203, "d": 204}
equiv = {"s": "0", "p": "1", "d": "2"}

# Read data
data = []
for omp in omps:
    aux = {}
    for l, fort in waves.items():
        aux[l] = phys.utils.parse_txt(f"./{omp}/fort.{waves[l]}")
    data.append(aux)

x = np.linspace(0, 180, 300)

colors = {"s": "crimson", "p": "forestgreen", "d": "dodgerblue"}
ls = {"s": "-", "p": "--", "d": "-."}

fig, ax = plt.subplots(figsize=(6, 4), constrained_layout=True)
for l in waves.keys():
    aux = []
    for idx, omp in enumerate(omps):
        spe = phys.utils.create_spline3(data[idx][l][:, 0], data[idx][l][:, 1])
        y = spe(x)
        aux.append(y)
        if idx == 1:
            ax.plot(x, y, color=colors[l], label=f"$l = {equiv[l]}$", ls=ls[l])
    ax.fill_between(x, aux[0], aux[1], alpha=0.2, color=colors[l], ec="none")

ax.legend()
# for i, omp in enumerate(omps):
#     for l, dat in data[i].items():
#         x = np.linspace(dat[:, 0].min(), dat[:, 0].max(), 300)
#         spe = phys.utils.create_spline3(dat[:, 0], dat[:, 1])
#         y = spe(x)
#         ax.plot(x, y, label=f"{omp} l = {l}")

ax.set_xlim(0, 80)
ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
ax.set_ylabel(r"$d\sigma/d\Omega$ [mb/sr]")
# ax.set_yscale("log")

fig.savefig("./cross_sections_Ebeam_45AMeV_Ex_17MeV.pdf", dpi=300)
fig.savefig("./cross_sections_Ebeam_45AMeV_Ex_17MeV.png", dpi=300)

plt.show()
