import pyphysics as phys
import numpy as np
from scipy.interpolate import CubicSpline
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

data = phys.parse_txt("./signal.dat")

# Ensure strictly increasing x
x = data[:, 0]
y = data[:, 1]
idx = np.argsort(x)
x = x[idx]
y = y[idx]

# Remove any duplicate x values (keep first)
unique_mask = np.concatenate(([True], np.diff(x) > 0))
x = x[unique_mask]
y = y[unique_mask]

# Build spline
spline = CubicSpline(x, y)
x_smooth = np.linspace(x.min(), x.max(), 500)

fig, ax = plt.subplots(1, 1, figsize=(4.5, 3.5), constrained_layout=True)
ax.plot(x_smooth, spline(x_smooth), marker=".", ls="", color="black", ms=2.5)
ax.set_xlabel("Time bucket")
ax.set_ylabel("Pulse height [ch]")
ax.set_xlim(0)
ax.set_ylim(0, 2400)

# Draw values
vals = [(252.87, 487.98), (294.96, 993.15), (384.94, 2030.96)]

for i, (a, b) in enumerate(vals):
    # Vertical
    ax.plot([a, a], [0, b], ls="--", lw=0.75, color="black")
    # Horizontal
    ax.plot([0, a], [b, b], ls="--", lw=0.75, color="black")

    # Time annotaion
    ax.annotate(f"$Z_{i + 1}$", xy=(a + 15, 150), ha="center", va="center", fontsize=12)
    # Charge annotaion
    ax.annotate(
        f"$Q^{{raw}}_{i + 1}$", xy=(35, b + 75), ha="center", va="center", fontsize=12
    )

# Baseline
rec = Rectangle(xy=(5, 330), width=50, height=110, fc="none", ec="black", lw=1, ls="--")
ax.add_patch(rec)
ax.annotate(
    r"$\bar{Q}_{BL}$",
    xy=(27, 331),
    xytext=(80, 177),
    ha="center",
    va="center",
    fontsize=12,
    arrowprops=dict(arrowstyle="-"),
)
fig.savefig("./Outputs/signal.pdf", dpi=300)

plt.show()


# def noisy_gaussian(x):
#     amp = 4000
#     mean = 50
#     sigma = 5
#     baseline = 20
#     noise = 10
#     gaussian = (amp * np.sqrt(np.pi) * sigma) * norm.pdf(x, loc=mean, scale=sigma)
#     noise = np.random.uniform(baseline - noise, baseline + noise, size=len(x))
#     return gaussian + noise


# x = np.linspace(0, 150, 1000)
# y = noisy_gaussian(x)

# fig, ax = plt.subplots(1, 1, figsize=(4.5, 3.5))
# ax: mplaxes.Axes
# ax.plot(x, y, color="black", lw=1.25)
# ax.set_xlabel("Time bucket")
# ax.set_ylabel("Pulse height [channel]")
# # Axis settings
# ax.spines["top"].set_visible(False)
# ax.spines["right"].set_visible(False)
# ax.tick_params(which="both", top=False, right=False)
# ax.set_ylim(0)
# ax.set_xlim(0)
# ax.locator_params(axis="y", nbins=5)
# # Annotations
# ymax = np.max(y)
# xmax = x[np.argmax(y)]
# kwargs = {"color": "dimgray", "lw": 1, "ls": "dashed"}
# ax.vlines(xmax, ymin=0, ymax=ymax, **kwargs)
# ax.hlines(ymax, xmin=0, xmax=xmax, **kwargs)
# arrow = {"fontsize": 14, "arrowprops": dict(arrowstyle="-")}
# ax.annotate(
#     "Z coordinate",
#     xy=(50, 0),
#     xytext=(80, 1e3),
#     **arrow
# )
# ax.annotate("Q of voxel", xy=(0, ymax), xytext=(5, 2e3), **arrow)

# fig.tight_layout()
# fig.savefig("./Outputs/signal.pdf", dpi=300)

# plt.show()
