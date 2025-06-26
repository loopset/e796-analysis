from typing import List
import pyphysics as phys
import uproot
import hist
import matplotlib.pyplot as plt

# Columns to read
cols = ["Ex", "EVertex", "fThetaLight", "ThetaCM"]
# Only f0
f0 = uproot.open(
    "../RootFiles/Pipe3/tree_20O_2H_3H_front_juan_RPx.root:Sel_Tree"
).arrays(cols)  # type: ignore
# Both layers
f01 = uproot.open("./Outputs/tree_ex_twosils_2H_3H.root:Sel_Tree").arrays(cols)  # type: ignore
dfs = [f0, f01]

# Draw Ex and Kin results
mKin = ((200, 0, 60), (200, 0, 25))
mEx = (150, -5, 25)

hsEx: List[hist.BaseHist] = []
hsKin: List[hist.BaseHist] = []
for df in dfs:
    hEx = hist.Hist.new.Reg(*mEx, label="E$_{x}$ [MeV]").Double()
    hEx.fill(df["Ex"])
    hKin = (
        hist.Hist.new.Reg(*mKin[0], label=r"$\theta_{\text{Lab}}$ [$\circ$]")
        .Reg(*mKin[1], label=r"E$_{\text{Lab}}$ [MeV]")
        .Double()
    )
    hKin.fill(df["fThetaLight"], df["EVertex"])
    hsEx.append(hEx)
    hsKin.append(hKin)

fig, axs = plt.subplots(1, 2, figsize=(9, 6))
ax = axs[0]

labels = ["One layer", "Two layers"]

for i, h in enumerate(hsEx):
    h.plot(ax=ax, yerr=False, flow="none", lw=1.5, label=labels[i])
ax.legend()

ax = axs[1]
for i, h in enumerate(hsKin):
    h.plot(ax=ax, cmap="managua_r" if i == 0 else "plasma_r", cmin=1, label=labels[i], cbar=None)

fig.tight_layout()
fig.savefig("./Outputs/comp.png", dpi=300)
plt.show()
