import uproot
import hist
import matplotlib.pyplot as plt
import matplotlib as mpl

plt.style.use("../../Python/actroot.mplstyle")

# Read the data
tree = uproot.open("../../Macros/PID/twopid.root:Simple_Tree")
arrays = tree.arrays()

# Histogram
h = hist.Hist(
    hist.axis.Regular(600, 0, 40, name="E1"),
    hist.axis.Regular(600, 0, 40, name="E0"),
)

# Fill
h.fill(arrays.E1, arrays.E0)

# Plot
fig, ax = plt.subplots(1, 1, figsize=(8, 5))
h.plot2d(ax=ax, cmap="managua_r", norm=mpl.colors.LogNorm(), flow=None, rasterized=True)
ax.set_xlabel(r"E$_{1}$ [MeV]", loc="right")
ax.set_ylabel(r"E$_{0}$ [MeV]", loc="top")
ax.annotate(r"$\Delta$E-E\\Sil mult = 1\\No track rec", xy=(0.8, 0.85), xycoords="axes fraction", ha="center", va="center", fontsize=16)

plt.tight_layout()
plt.savefig("./Outputs/twosils.pdf", dpi=200)
plt.show()