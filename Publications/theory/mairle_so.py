import pyphysics as phys
import matplotlib.pyplot as plt
import numpy as np

# Read data
data = [phys.parse_txt(f"./Inputs/mairle/n{i}.txt") for i in range(1, 4)]


# Interpolation function
def so(a, n):
    return 23.27 * np.pow(a, -0.583) / n


# X axis
x = np.linspace(10, 250, 200)

fig, ax = plt.subplots()
for i, d in enumerate(data):
    plot = ax.plot(
        d[:, 0], d[:, 1], ls="none", marker="o", mfc="none", label=f"n = {i}"
    )
    ax.plot(x, so(x, i + 1), color=plot[0].get_color(), marker="none")

# Annotation
ax.annotate(
    r"$\xi = \frac{23.27}{\text{n}}\; \text{A}^{-0.583}$",
    xy=(0.5, 0.75),
    xycoords="axes fraction",
    ha="center",
    va="center",
    fontsize=16,
)

# 35Si case
ax.annotate(
    r"$^{35}$Si",
    xy=(34.36, 0.784),
    xytext=(15, 0.6),
    ha="center",
    va="center",
    fontsize=12,
    arrowprops=dict(arrowstyle="-"),
)

# Legend
ax.legend(title="Number of nodes")
# Axis settings
ax.set_xlabel("A")
ax.set_ylabel(r"$\xi\equiv\frac{2\epsilon_{\text{so}}}{2\ell + 1}$ [MeV]")
fig.tight_layout()
fig.savefig("./Outputs/mairle_so.pdf")
plt.show()
