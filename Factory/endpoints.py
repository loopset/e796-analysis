import sys
import time

sys.path.append("../Python/")
from interfaces import TPCDataInterface

from collections import Counter
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import networkx as nx
from scipy.spatial import KDTree
from sklearn.decomposition import PCA
from collections import defaultdict

data = TPCDataInterface("./Inputs/base.root", [1])

# fig, ax = plt.subplots(1, 1, figsize=(7, 5))
# plt.sca(ax)
# data.draw()
# plt.show()

# Create points on 2D
voxels = np.array([v[:2] for v in data.voxels])
binning = 2


def downscale(voxels, factor):
    grid = np.zeros((128, 128))
    for v in voxels:
        grid[int(v[0]), int(v[1])] += 1
    new_shape = (128 // factor, 128 // factor)
    binned = grid.reshape(new_shape[0], factor, new_shape[1], factor).mean(axis=(1, 3))
    ret = np.array(np.nonzero(binned)).T
    binned[binned == 0] = np.nan
    return ret, binned


voxels, grid = downscale(voxels, binning)


# Sort as distances to origin of coordinates
dists = np.sqrt(np.sum(voxels**2, axis=1))
sort_idxs = np.argsort(dists)
voxels = voxels[sort_idxs]

start = time.time()


# Create graph
def create_graph(voxels: np.ndarray) -> nx.Graph:
    g = nx.Graph()
    for i, pos in enumerate(voxels):
        g.add_node(i, pos=pos)

    # Add edges from KDTree with distance threshold
    tree = KDTree(voxels)
    pairs = tree.query_pairs(1)
    for i, j in pairs:
        g.add_edge(i, j)
    return g


g = create_graph(voxels)


# Determine endpoints
def find_endpoints(g: nx.Graph) -> list:
    candidates = g.nodes
    # Sort by num of endpoints
    candidates = sorted(candidates, key=lambda i: g.degree(i))
    # Let's use the first 8 elements
    a = candidates[:8]
    b = candidates[-4:]
    return list(set(a + b))


endpoints = find_endpoints(g)


# Shortest path OR longest one
def track_eval(g: nx.Graph, endp: list) -> list:
    best_track = []
    best_goodness = -1
    largest_distance = -1

    for i, start in enumerate(endp):
        for end in endp[i + 1 :]:
            try:
                path = nx.shortest_path(g, start, end)
            except nx.NetworkXNoPath:
                continue
            path_points = np.array([g.nodes[i]["pos"] for i in path])
            if path_points.shape[0] < 7:
                continue
            pca = PCA(n_components=2)
            pca.fit(path_points)
            goodness = pca.explained_variance_[0] / sum(pca.explained_variance_)

            if goodness > best_goodness:
                best_goodness = goodness
                best_track = path
            # p0 = g.nodes[start]["pos"]
            # p1 = g.nodes[end]["pos"]
            # dist = np.linalg.norm(p0 - p1)
            # if dist > largest_distance:
            #     largest_distance = dist
            #     best_track = path

    return best_track


track = track_eval(g, endpoints)


def common_path(g: nx.Graph, endp: list) -> dict:
    paths = []
    for i, start in enumerate(endp):
        for end in endp[i + 1 :]:
            try:
                path = nx.shortest_path(g, start, end)
                paths.append(path)
            except nx.NetworkXNoPath:
                continue
    # Count ocurrences
    counter = Counter(node for path in paths for node in path)
    print(counter)
    return counter


common = common_path(g, endpoints)


def build_common(d: dict, thresh: int) -> list:
    ret = []
    for node, count in d.items():
        if count >= thresh:
            ret.append(node)
    return ret


track_in_common = build_common(common, 10)

end = time.time()
print(f"Elapsed time: {end - start}")

best_xy = np.array([g.nodes[p]["pos"][:2] for p in track])
best_xy_common = np.array([g.nodes[p]["pos"][:2] for p in track_in_common])

fig, axs = plt.subplots(1, 2, figsize=(12, 5))
ax1, ax2 = axs
ax1: plt.Axes
ax2: plt.Axes
plt.sca(ax1)
ax1.imshow(grid.T, cmap="managua_r", origin="lower")
# data.draw()
for endp in endpoints:
    pos = g.nodes[endp]["pos"]
    ax1.scatter(pos[0], pos[1], color="orange", marker="*", s=75, zorder=4)
ax1.scatter(best_xy[:, 0] + 0.5, best_xy[:, 1] + 0.5, color="red")
# ax1.scatter(best_xy_common[:, 0] + 0.5, best_xy_common[:, 1] + 0.5, color="dodgerblue")
#########
nx.draw(g, ax=ax2, node_size=10)

plt.show()
