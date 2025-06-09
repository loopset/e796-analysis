import pyphysics as phys
from pyphysics.actroot_interface import SFInterface
import matplotlib.pyplot as plt

## 1-> Systematic uncertainty from (p,p) normalization
pp = SFInterface("../../Fits/pp/Outputs/sfs.root")

## 2-> Systematic uncertainty from (d,t) OMPs
dtgs = phys.parse_txt("../../Fits/dt/Outputs/xs/g0_xs.dat", 3)
comp = phys.Comparator(dtgs)
files = {
    "Daeh+Pang": "../../Fits/dt/Inputs/Sys/Daeh_Pang/fort.202",
    "Daeh+HT1p": "../../Fits/dt/Inputs/Sys/Daeh_HT1p/fort.202",
    "Haixia+Pang": "../../Fits/dt/Inputs/Sys/Haixia_Pang/fort.202",
    "Haixia+HT1p": "../../Fits/dt/Inputs/Sys/Haixia_HT1p/fort.202",
}
for key, file in files.items():
    comp.add_model(key, file)
comp.fit()

fig, axs = plt.subplots(1, 2)

# Comparator
ax = axs[0]
comp.draw(ax=ax)

fig.tight_layout()

plt.show()