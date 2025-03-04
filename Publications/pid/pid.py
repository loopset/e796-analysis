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
files = ["./Inputs/pid_front.root", "./Inputs/pid_side.root"]
arrays = []
for file in files:
    root = uproot.open(f"{file}:PID_Tree")
    arrays.append(root.arrays())

# Init histograms
hists = []
for _ in files:
    hists.append(
        hist.Hist(
            hist.axis.Regular(400, 0, 40, name="ESil"),
            hist.axis.Regular(800, 0, 2000, name="Qave"),
        )
    )

for i, array in enumerate(arrays):
    hists[i].fill(array["ESil0"], array["fQave"])

# # triton cut
# with r.TFile("../../PostAnalysis/Cuts/LightPID/pid_2H.root") as f:
#     cut = f.Get("CUTG")
#     print(cut)


# def fit_cut(h, cut):
#     inside = []
#     xc = h.axes[0].centers
#     yc = h.axes[1].centers
#     contents = h.view()
#     for i, x in enumerate(xc):
#         for j, y in enumerate(yc):
#             content = contents[i, j]
#             if content > 0 and cut.IsInside(x, y):
#                 inside.append({"x": x, "y": y, "c": content})
#     array = ak.Array(inside)
#     coeffs = np.polyfit(array.x, array.y, deg=3, w=1.0 / array.c**2)
#     xfit = np.linspace(0, 20, 100)
#     return xfit, coeffs


# xfit, coeffs = fit_cut(hists[0], cut)


# def eval(x, coeffs, off):
#     copy = coeffs.to_numpy()
#     copy[-1] += off
#     return np.polyval(copy, x)


# yfit = eval(xfit, coeffs, 100)


# Plot
fig, axs = plt.subplots(1, 2, figsize=(12, 6))
for i, h in enumerate(hists):
    ax = axs[i]
    h.plot2d(ax=ax, cmap="managua_r", cmin=1, flow=None, rasterized=True)
    ax.set_xlabel(r"E$_{\textrm{\normalsize Sil}}$ [MeV]", loc="right")
    ax.set_ylabel(r"$\bar{Q}$ [mm$^{-1}$]", loc="top")
    if i == 1:  # side plot
        ax.set_xlim(0, 20)
        ax.set_ylim(0, 900)
## annotations
# front
ax = axs[0]
# Axis
ax.annotate(r"\textbf{Transfer}", xy=(0.8, 0.85), xycoords="axes fraction", ha="center", va="center", fontsize=16)
# Bananas
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

labels = ["p", "d", "t", "3He", r"$\alpha$"]
poss = [(7.4, 190), (9.77, 261), (11.1, 336), (24, 590), (30.5, 615)]
d = 20
a = -75
ds = [d, d, d, 200, 200]
ass = [a, a, a, -90, -90]
for i, l in enumerate(labels):
    annotate(l, poss[i], ds[i], ass[i])

# side
ax = axs[1]
ax.annotate(r"\textbf{(In)elastic}", xy=(0.8, 0.85), xycoords="axes fraction", ha="center", va="center", fontsize=16)

# Bananas
labels = ["p", "d"]
poss = [(6.62, 150), (8.8, 240)]
d = 20
a = -75
for l, p in zip(labels, poss):
    annotate(l, p, d, a)

plt.tight_layout()
plt.savefig("./Outputs/pid.pdf", dpi=200)
plt.show()
