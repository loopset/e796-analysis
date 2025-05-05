import pyphysics as phys
import uproot
import hist
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt

exp = uproot.open(
    "../../PostAnalysis/RootFiles/Pipe3/tree_20O_2H_3H_front_juan_RPx.root:Sel_Tree"
).arrays(["EVertex", "fThetaLight"])  # type: ignore

# Histogram
xmodel = (200, 0, 60)
ymodel = (200, 0, 20)
h = (
    hist.Hist.new.Reg(*xmodel, label=r"$\theta_{\mathrm{Lab}}$ [$^{\circ}$]")
    .Reg(*ymodel, label=r"E$_{\mathrm{Lab}}$ [MeV]")
    .Double()
)
h.fill(exp["fThetaLight"], exp["EVertex"])

# Theoretical kinematics
kins = [phys.KinInterface("20O(d,t)@700|0")]

fig, ax = plt.subplots(1, 1, figsize=(7, 5))
ax: mplaxes.Axes
h.plot(cmin=1, cmap="managua_r")
for kin in kins:
    kin.plot_kin3(lw=1.25)

plt.gca().yaxis.set_major_locator(plt.MaxNLocator(integer=True, nbins=5))  # type: ignore
fig.tight_layout()
plt.show()
