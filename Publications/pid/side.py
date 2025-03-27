import cmath
import uproot
import awkward as ak
import hist
import matplotlib as mpl
import matplotlib.pyplot as plt
import ROOT as r
import numpy as np

plt.style.use("../../Python/actroot.mplstyle")

# Read files
files = ["./Inputs/pid_side.root"]
root = uproot.open(f"{files[0]}:PID_Tree")
data = root.arrays()

# Init histograms
hpid = hist.Hist(
    hist.axis.Regular(400, 0, 40, name="ESil"),
    hist.axis.Regular(800, 0, 2000, name="Qave"),
)

hpid.fill(data["ESil0"], data["fQave"])


# Plot
fig, ax = plt.subplots(1, 1, figsize=(4, 3.5))
ax: plt.Axes
hpid.plot2d(ax=ax, cmap="managua_r", cmin=1, flow=None, rasterized=True)
ax.set_xlabel(r"E$_{\textrm{\normalsize Sil}}$ [MeV]")
ax.set_ylabel(r"$\bar{Q}$ [mm$^{-1}$]")
ax.set_xlim(0, 17)
ax.set_ylim(0, 900)
# Annotations
def annotate(label: str, pos: tuple, l: float, a: float):
    xt = pos[0] + l * np.cos(np.radians(a))
    yt = pos[1] + l * np.sin(np.radians(a))
    ax.annotate(
        label,
        xy=pos,
        xytext=(xt, yt),
        fontsize=16,
        ha="center",
        va="center",
        arrowprops=dict(arrowstyle="-"),
    )

# side
ax.annotate(
    r"\textbf{(In)elastic}",
    xy=(0.8, 0.85),
    xycoords="axes fraction",
    ha="center",
    va="center",
    fontsize=16,
)

# Bananas
labels = ["p", "d"]
poss = [(6.62, 150), (8.8, 240)]
d = 20
a = -75
for l, p in zip(labels, poss):
    annotate(l, p, d, a)

plt.tight_layout()
plt.savefig("./Outputs/pid_side.png", dpi=200)
plt.show()
