import pyphysics as phys
import uproot
import hist
import matplotlib as mpl
import matplotlib.ticker as mpltick
import matplotlib.axes as mplaxes
import matplotlib.pyplot as plt

exp = uproot.open(
    "../../PostAnalysis/RootFiles/Pipe3/tree_20O_2H_3H_front_juan_RPx.root:Sel_Tree"
).arrays(["EVertex", "fThetaLight"])  # type: ignore

# Histogram
xmodel = (200, 0, 60)
ymodel = (200, 0, 20)
h = (
    hist.Hist.new.Reg(*xmodel, label=r"$\theta_{\mathrm{Lab}}$ [$\circ$]")
    .Reg(*ymodel, label=r"E$_{\mathrm{Lab}}$ [MeV]")
    .Double()
)
h.fill(exp["fThetaLight"], exp["EVertex"])

# Theoretical kinematics
labels = ["g.s", r"E$_{\text{x}}$ = 15 MeV"]
kins = [phys.Kinematics("20O(d,t)@700"), phys.Kinematics("20O(d,t)@700|15")]

fig, ax = plt.subplots(1, 1, figsize=(5, 5))
ax: mplaxes.Axes
h.plot(cmin=1, cmap="managua_r")
cmap = mpl.colormaps["Dark2"].colors #type: ignore
ax.set_prop_cycle(color=cmap)
for i, kin in enumerate(kins):
    x, y = kin.get_line3()
    ax.plot(x, y, lw=1.25, label=labels[i], ls="--")

ax.legend()
ax.yaxis.set_major_locator(mpltick.MaxNLocator(integer=True, nbins=5))
fig.tight_layout()
fig.savefig("./Outputs/kin.pdf", dpi=300)
fig.savefig("/media/Data/Docs/EuNPC/figures/kin.png", dpi=600)
plt.show()
