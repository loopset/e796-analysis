import pyphysics as phys
import numpy as np
from scipy.stats import norm
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes


def noisy_gaussian(x):
    amp = 4000
    mean = 50
    sigma = 5
    baseline = 100
    noise = 50
    gaussian = (amp * np.sqrt(np.pi) * sigma) * norm.pdf(x, loc=mean, scale=sigma)
    noise = np.random.uniform(baseline - noise, baseline + noise, size=len(x))
    return gaussian + noise


x = np.linspace(0, 150, 1000)
y = noisy_gaussian(x)

fig, ax = plt.subplots(1, 1, figsize=(4.5, 4.5))
ax: mplaxes.Axes
ax.plot(x, y, color="black", lw=1.25)
ax.set_xlabel("Time bucket")
ax.set_ylabel("Pulse height [fC]")
# Axis settings
ax.spines["top"].set_visible(False)
ax.spines["right"].set_visible(False)
ax.tick_params(which="both", top=False, right=False)
ax.set_ylim(0)
ax.set_xlim(0)
ax.locator_params(axis="y", nbins=5)
# Annotations
ymax = np.max(y)
xmax = x[np.argmax(y)]
kwargs = {"color": "dimgray", "lw": 1, "ls": "dashed"}
ax.vlines(xmax, ymin=0, ymax=ymax, **kwargs)
ax.hlines(ymax, xmin=0, xmax=xmax, **kwargs)
arrow = {"fontsize": 14, "arrowprops": dict(arrowstyle="-")}
ax.annotate(
    "Z coordinate",
    xy=(50, 0),
    xytext=(80, 1e3),
    **arrow
)
ax.annotate("Q of voxel", xy=(0, ymax), xytext=(5, 2e3), **arrow)

fig.tight_layout()
fig.savefig("./Outputs/signal.pdf")

plt.show()
