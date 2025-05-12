import pyphysics as phys
import uproot
import awkward as ak
import hist
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltic
import ROOT as r
import numpy as np

plt.rcParams.update(
    {
        "font.size": 20,
        "axes.titlesize": 22,
        "axes.labelsize": 22,
        "xtick.labelsize": 20,
        "ytick.labelsize": 20,
    }
)

# Read files
files = ["./Inputs/pid_front.root", "./Inputs/pid_side.root"]
arrays = []
for file in files:
    root = uproot.open(f"{file}:PID_Tree")
    arrays.append(root.arrays())  # type: ignore

# Init histograms
hists = []
for _ in files:
    hists.append(
        hist.Hist.new.Reg(400, 0, 40, name="ESil", label=r"E [MeV]")
        .Reg(800, 0, 2000, name="Qave", label=r"$\Delta$E [arb. unit]")
        .Double()
    )

for i, array in enumerate(arrays):
    if i == 1:
        mask = ~np.isin(array["NSil0"], [6, 7])  # to avoid double bananas in SIDE LAYER
        x = array["ESil0"][mask]
        y = array["fQave"][mask]
    else:
        x = array["ESil0"]
        y = array["fQave"]
    hists[i].fill(x, y)

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
    if i == 1:  # side plot
        ax.set_xlim(0, 15)
        ax.set_ylim(0, 800)
## annotations
# front
ax: mplaxes.Axes = axs[0]
# Axis
ax.annotate(
    r"\noindent\textbf{a)}\newline\newline\textbf{Front}",
    xy=(0.8, 0.85),
    xycoords="axes fraction",
    ha="center",
    va="center",
)


# Bananas
def annotate(label: str, pos: tuple, l: float, a: float):
    xt = pos[0] + l * np.cos(np.radians(a))
    yt = pos[1] + l * np.sin(np.radians(a))
    ax.annotate(
        label,
        xy=pos,
        xytext=(xt, yt),
        ha="center",
        va="center",
        arrowprops=dict(arrowstyle="-"),
    )


labels = ["p", "d", "t", "3He", r"$\alpha$"]
poss = [(7.4, 190), (9.77, 261), (11.1, 300), (24, 590), (30.5, 615)]
d = 20
a = -75
ds = [d, d, d, 200, 200]
ass = [a, a, a, -90, -90]
for i, l in enumerate(labels):
    annotate(l, poss[i], ds[i], ass[i])

# side
ax = axs[1]
ax.annotate(
    r"\noindent\textbf{b)}\newline\newline\textbf{Side}",
    xy=(0.8, 0.85),
    xycoords="axes fraction",
    ha="center",
    va="center",
)

# Bananas
labels = ["p", "d"]
poss = [(6.62, 175), (8.8, 240)]
d = 15
a = -75
for l, p in zip(labels, poss):
    annotate(l, p, d, a)

ax.xaxis.set_major_locator(mpltic.MaxNLocator(nbins=4, integer=True))

## General settings
for ax in axs.flat:
    ax.locator_params(axis="y", nbins=4)

fig.tight_layout()
fig.subplots_adjust(wspace=0.05)
fig.savefig("./Outputs/pid.pdf", dpi=200)
fig.savefig("./Outputs/pid.eps", dpi=200)
plt.show()
