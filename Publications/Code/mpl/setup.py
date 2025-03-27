from turtle import color
import matplotlib.pyplot as plt
import matplotlib as mpl
from mpl_toolkits.mplot3d.art3d import Poly3DCollection, Line3DCollection

import numpy as np

import ROOT as r

plt.style.use("../../../Python/actroot.mplstyle")
plt.rcParams["lines.marker"] = ""
plt.rcParams["savefig.pad_inches"] = 0.15

import util as u

# Read TPC data
data = u.DataInterface("../Events/run_155_entry_630.root")

fig = plt.figure(figsize=(9, 6))
ax = fig.add_subplot(111, projection="3d")
data.draw_3d()
axmax = 200
ax.set_xlim(0, axmax)
ax.set_ylim(0, axmax)
ax.set_zlim(0, axmax)

# Draw silicons
specs = r.ActPhysics.SilSpecs()
specs.ReadFile("../../../configs/detailedSilicons.conf")


def draw_layers(specs, zoffset=50):
    colors = {"f0": "bisque", "f1": "khaki", "l0": "orchid"}
    for name, layer in specs.GetLayers():
        print(name)
        verts = []
        unit = layer.GetUnit()
        point = layer.GetPoint()
        side = "front" if "f" in name else "left"
        for idx, graph in layer.GetSilMatrix().GetGraphs():
            sil = []
            for i in range(graph.GetN()):
                a = graph.GetPointX(i)
                b = graph.GetPointY(i) + zoffset
                ## Convert to pad units
                a /= 2
                b /= 2.42
                if side == "front":
                    sil.append((point.X(), a, b))
                else:
                    sil.append((a, point.Y(), b))
            verts.append(sil)
        # Poly collection 3D
        poly = Poly3DCollection(
            np.array(verts),
            facecolors=colors[name],
            linewidths=0.5,
            edgecolors="black",
            linestyles="solid",
        )
        plt.gca().add_collection3d(poly)


draw_layers(specs, 75)


# Draw ACTAR
def draw_actar():
    size = 128
    vertices = np.array(
        [
            [0, 0, 0],
            [size, 0, 0],
            [size, size, 0],
            [0, size, 0],  # Bottom square
            [0, 0, size],
            [size, 0, size],
            [size, size, size],
            [0, size, size],  # Top square
        ]
    )
    edges = [
        (0, 1),
        (1, 2),
        (2, 3),
        (3, 0),  # Bottom
        (4, 5),
        (5, 6),
        (6, 7),
        (7, 4),  # Top
        (0, 4),
        (1, 5),
        (2, 6),
        (3, 7),
    ]  # Vertical connections
    edge_lines = [(vertices[start], vertices[end]) for start, end in edges]
    poly = Line3DCollection(edge_lines, colors="dodgerblue", linewidths=1.5)
    plt.gca().add_collection3d(poly)
draw_actar()

# Draw beam line
ax.quiver(75, 5, 20, 50, 0, 0, color='dodgerblue', linewidth=2, arrow_length_ratio=0.2)  # Draw the arrow
ax.text(75, 10, 5, 'Beam direction', color='dodgerblue', fontsize=12, ha='center', va="center")  # Add text under the arrow

pad = -5
ax.set_xlabel("X [pads]", labelpad=pad)
ax.set_ylabel("Y [pads]", labelpad=pad)
ax.set_zlabel("Z [time buckets]", labelpad=pad)
# Disable axis
# ax.set_axis_off()
ax.set_xticks([])
ax.set_yticks([])
ax.set_zticks([])
ax.xaxis.set_pane_color((1, 1, 1, 0))
ax.yaxis.set_pane_color((1, 1, 1, 0))
ax.zaxis.set_pane_color((1, 1, 1, 0))
# ax.w_xaxis.pane.set_visible(False)
# ax.w_yaxis.pane.set_visible(False)
# ax.w_zaxis.pane.set_visible(False)
# ax.set_frame_on(False)
ax.grid(False)

# View
# ax.view_init(elev=20, azim=-130)
ax.view_init(elev=35, azim=-132)

plt.tight_layout()
# plt.savefig(("./Outputs/setup.pdf"))
plt.savefig("./Outputs/setup.png")
plt.show()
