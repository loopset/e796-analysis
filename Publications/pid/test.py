import awkward as ak
import uproot
import hist
import matplotlib.pyplot as plt

tree = uproot.open("../../Macros/PID/twopid.root:Simple_Tree")
arrays= tree.arrays()
layers = arrays["Layers"]
energies = arrays["Energies"]
# threshold cut
thresh = energies >= 0.5

layersThres = layers[thresh]
energiesThres = energies[thresh]

# final cut
mult = (ak.num(layersThres) == 2) & ak.any(layersThres == "f0", axis=1) & ak.any(layersThres == "f1", axis=1)
final = energiesThres[mult]

h = hist.Hist(
    hist.axis.Regular(400, 0, 40, name="E0"),
    hist.axis.Regular(400, 0, 40, name="E1"),
)
h.fill(final[:, 0], final[:, 1])

fig, axs = plt.subplots(1, 1, figsize=(12, 6))
h.plot2d(ax=axs, cmap="managua_r", cmin=1, flow=None)
plt.show()