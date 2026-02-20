from cProfile import label

from matplotlib.lines import Line2D
import pyphysics as phys
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from matplotlib.patches import Rectangle
import matplotlib.colors as mplcolor
import numpy as np

# Declare cluster
n = 20
cluster = np.full((n, n), np.nan)
# start/end (y, x)
y0, x0 = 5, 5
y1, x1 = 15, 15
ys = np.linspace(y0, y1, 80).astype(int)
xs = np.linspace(x0, x1, 80).astype(int)
w = 1  # thickness
for y, x in zip(ys, xs):
    cluster[max(0, y - w) : min(n, y + w + 1), max(0, x - w) : min(n, x + w + 1)] = 1
# Cut
ycut = 12
cluster[ycut, :] = np.nan
# Mask remaining voxels in lower cluster
cluster[10:12, 12:] = np.nan
# Upper part
cluster[13:15, :12] = np.nan
# cluster[12,13:] = np.nan
# cluster[13:,12] = np.nan
# cluster[:,13] = np.nan


def draw_cluster(ax: Axes, grid: np.ndarray, **kwargs):
    ax.imshow(grid, origin="lower", interpolation="none", cmap="managua_r", **kwargs)


def draw_grid(ax: Axes):
    for (y, x), val in np.ndenumerate(cluster):
        if not np.isnan(val):
            rect = Rectangle(
                (x - 0.5, y - 0.5),
                1,
                1,
                edgecolor="gray",
                facecolor="none",
                linewidth=0.5,
            )
            ax.add_patch(rect)


def apply_search(
    ax: Axes,
    grid: np.ndarray,
    seed: tuple,
    markerseed="o",
    gridcolor="crimson",
    only_seed=False,
) -> np.ndarray:
    ax.plot(*seed, marker=markerseed, markersize=5, mec="deeppink", mfc="white")

    clone = grid.copy()

    if only_seed:
        return clone

    # Draw grid
    y_range = np.arange(max(0, seed[1] - w), min(n, seed[1] + w + 1))
    x_range = np.arange(max(0, seed[0] - w), min(n, seed[0] + w + 1))

    # vectorized patch creation
    for ny in y_range:
        for nx in x_range:
            isNew = False
            hatch = None
            if grid[ny, nx] == 1 and not (nx, ny) == seed:
                isNew = True
                hatch = "...."
            rect = Rectangle(
                (nx - 0.5, ny - 0.5),
                1,
                1,
                edgecolor=gridcolor,
                facecolor=mplcolor.to_rgba(gridcolor, alpha=0.35) if isNew else "none",
                linewidth=1,
                ##hatch=hatch,
            )
            ax.add_patch(rect)
            # And update grid
            if (nx, ny) == seed or hatch is not None:
                clone[nx, ny] = 2
    return clone


def merge_grids(grids: list) -> np.ndarray:
    return np.nanmax(np.array(grids), axis=0)


fig, axs = plt.subplots(2, 2, figsize=(5, 5), constrained_layout=True)

# Starting clusters
ax: Axes = axs.flat[0]
draw_cluster(ax, cluster)
draw_grid(ax)
# Seed
it0 = apply_search(ax, cluster, (4, 4))
# Annotate
ax.legend(
    handles=[
        Line2D(
            [],
            [],
            marker="o",
            ls="none",
            ms=5,
            mfc="white",
            mec="hotpink",
            label="Seed",
        )
    ],
    handletextpad=0.0,
    handlelength=1.25,
    fontsize=12,
    loc=(0.485, 0.15),
)
ax.annotate(
    "Search grid",
    xy=(5.5, 2.5),
    xytext=(11, 1.5),
    ha="center",
    va="center",
    fontsize=12,
    color="crimson",
    arrowprops=dict(arrowstyle="->", color="crimson"),
)

# Second generation
ax = axs.flat[1]
draw_cluster(ax, it0)
draw_grid(ax)
seeds = [(4, 5), (5, 5), (5, 4)]
outs = []
for seed in seeds:
    outs.append(apply_search(ax, it0, seed, only_seed=False))
it1 = merge_grids(outs)

# Third iteration
ax = axs.flat[2]
draw_cluster(ax, it1)
draw_grid(ax)
seeds = [(4, 6), (5, 6), (6, 6), (6, 5), (6, 4)]
outs = []
for seed in seeds:
    outs.append(apply_search(ax, it0, seed, only_seed=True))
it2 = merge_grids(outs)
ax.annotate(
    "",
    xy=(5, 6),
    xytext=(7, 8),
    arrowprops=dict(arrowstyle="<-", connectionstyle="arc3,rad=0.3", color="black"),
)
ax.annotate(
    "",
    xy=(6, 5),
    xytext=(8, 7),
    arrowprops=dict(arrowstyle="<-", connectionstyle="arc3,rad=-0.3", color="black"),
)

# Final
ax = axs.flat[-1]
final = cluster.copy()
final[~np.isnan(final)] = 2
# But draw second cluster
final[(~np.isnan(final)) & (np.arange(final.shape[0])[:, None] > 12)] = 1.5
draw_cluster(ax, final, vmin=1, vmax=2)
draw_grid(ax)
# Annotate
ax.annotate(
    "Cluster 1",
    xy=(14, 6),
    ha="center",
    va="center",
    fontsize=12,
    color="darkorange",
    # color=plt.get_cmap("managua_r")(0.9999),
)
ax.annotate(
    "Cluster 2",
    xy=(9, 16),
    ha="center",
    va="center",
    fontsize=12,
    color=plt.get_cmap("managua_r")(0.5),
)

# Common settings
for i, ax in enumerate(axs.flat):
    ax.annotate(
        chr(97 + i) + ")",
        xy=(0.1, 0.9),
        xycoords="axes fraction",
        ha="center",
        va="center",
        fontsize=14,
        fontweight="bold",
    )
    ax.set_xticks([])
    ax.set_yticks([])

fig.savefig("./Outputs/continuity.pdf", dpi=300)
plt.show()
