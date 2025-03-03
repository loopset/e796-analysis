import uproot
import awkward as ak
import hist
import matplotlib as mpl
import matplotlib.pyplot as plt

plt.style.use("../Code/mpl/actroot.mplstyle")

arrays = uproot.concatenate(
    "../../RootFiles/Corrector/*.root:ACTAR_Merged",
    ["fSilEs", "fSilLayers", "fSilNs", "fQave"],
    "fLightIdx != -1",
)

layers = arrays["fSilLayers"]

# f0
f0 = (ak.num(layers) == 1) & (ak.firsts(layers) == "f0")
# l0
l0 = (ak.num(layers) == 1) & (ak.firsts(layers) == "l0")

# Apply to others
qf0 = arrays["fQave"][f0]
ef0 = arrays["fSilEs"][f0][:, 0]
ql0 = arrays["fQave"][l0]
el0 = arrays["fSilEs"][l0][:, 0]

# Histogram!
# front
pidf0 = hist.Hist(
    hist.axis.Regular(400, 0, 40, name=r"$E_{Sil}$"),
    hist.axis.Regular(800, 0, 2000, name="Qave"),
)
pidf0.fill(ef0, qf0)
# side
pidl0 = hist.Hist(
    hist.axis.Regular(400, 0, 40, name="ESil"),
    hist.axis.Regular(800, 0, 2000, name="Qave"),
)
pidl0.fill(el0, ql0)

fig, axs = plt.subplots(1, 2, figsize=(12, 6))
pidf0.plot2d(ax=axs[0], cmap="managua_r", cmin=1, flow=None)
pidl0.plot2d(ax=axs[1], cmap="managua_r", cmin=1, flow=None)
# Manually settings labels
for ax in axs:
    ax.set_xlabel(r"$E_{Sil}$ [MeV]", loc="right")
    ax.set_ylabel(r"$\bar{Q}$ [mm$^{-1}$]", loc="top")
plt.show()
