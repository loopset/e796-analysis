import pyphysics as phys
import hist
import uproot

import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import matplotlib.ticker as mpltick

import sys

sys.path.append("../")
import styling as sty

# Read histograms
hs = []
files = [
    "../../Calibrations/Actar/Inputs/gain.root",
    "../../Calibrations/Actar/Outputs/aligned.root",
]
for file in files:
    h = uproot.open(file)["h"].to_hist()  # type: ignore
    h.axes[0].label = "Channel number"
    h.axes[1].label = "Charge [channels]"
    # Rebin
    h = h[:, ::2j]
    hs.append(h)


fig, axs = plt.subplots(1, 2, figsize=(8,4))

over = 3500
# Raw
phys.utils.set_hist_overflow(hs[0], over)
hs[0].plot(ax=axs[0], cmax=over, cbar=False, **sty.base2d)

# Aligned
phys.utils.set_hist_overflow(hs[1], over)
hs[1].plot(ax=axs[1], cmax=over, **sty.base2d)

phys.utils.annotate_subplots(axs)
# Save
fig.tight_layout()
fig.savefig(sty.thesis + "pad_alignment.pdf", dpi=300)
plt.show()
