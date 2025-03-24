import sys
import time

sys.path.append("../Python/")
from interfaces import TPCDataInterface

import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import networkx as nx
from scipy.spatial import KDTree
from sklearn.decomposition import PCA

data = TPCDataInterface("./Inputs/multifragmentation.root", [0])

# fig, ax = plt.subplots(1, 1, figsize=(7, 5))
# plt.sca(ax)
# data.draw()
# plt.show()

# Create points
voxels = np.array([v[:3] for v in data.voxels])
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
    pairs = tree.query_pairs(2)
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
    return candidates[:12]


endpoints = find_endpoints(g)


# Shortest path
def track_eval(g: nx.Graph, endp: list) -> list:
    best_track = None
    best_goodness = -1
    largest_distance = -1

    for i, start in enumerate(endp):
        for end in endp[i + 1 :]:
            path = nx.shortest_path(g, start, end)
            path_points = np.array([g.nodes[i]["pos"] for i in path])
            if path_points.shape[0] < 3:
                continue
            # pca = PCA(n_components=3)
            # pca.fit(path_points)
            # goodness = pca.explained_variance_[0] / sum(pca.explained_variance_)

            # if goodness > best_goodness:
            #     best_goodness = goodness
            #     best_track = path
            p0 = g.nodes[start]["pos"]
            p1 = g.nodes[end]["pos"]
            dist = np.linalg.norm(p0 - p1)
            if dist > largest_distance:
                largest_distance = dist
                best_track = path

    return best_track


track = track_eval(g, endpoints)

end = time.time()
print(f"Elapsed time: {end - start}")

best_xy = np.array([g.nodes[p]["pos"][:2] for p in track])

fig, axs = plt.subplots(1, 2, figsize=(12, 5))
ax1, ax2 = axs
ax1: plt.Axes
ax2: plt.Axes
plt.sca(ax1)
data.draw()
ax1.scatter(best_xy[:, 0] + 0.5, best_xy[:, 1] + 0.5, color="red")
#########
nx.draw(g, ax=ax2, node_size=10)

plt.show()
