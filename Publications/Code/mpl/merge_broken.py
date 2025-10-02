import pyphysics as phys
import matplotlib.pyplot as plt
from pyphysics.actroot_interface import TPCInterface
import ROOT as r

raw = r.TFile("../Events/merge_before.root").Get("TPCData") #type: ignore

tpc = TPCInterface(raw)

fig, ax = plt.subplots(subplot_kw={"projection": "3d"})
tpc.plot_3d()

fig.tight_layout()
plt.show()