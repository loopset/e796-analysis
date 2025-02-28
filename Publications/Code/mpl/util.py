import ROOT as r
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import cmcrameri.cm as cmc

class DataInterface():
    def __init__(self, file: str, dims: tuple = (128, 128, 128))-> None:
        self.voxels = []
        self.lines = []
        self.dims = dims
        self.xy = None
        self.xz = None
        self.d3 = None
        self.rp = None
        
        # By default, read XY projection
        self.parse(file)
        self.fill()

        return

    def parse(self, file: str) -> None:
        f = r.TFile(file)
        tpc = f.Get("TPCData")
        for v in tpc.fRaw:
            l = [v.GetPosition().X(), v.GetPosition().Y(), v.GetPosition().Z(), v.GetCharge(), -11]
            self.voxels.append(l)
        # Sort clusters from bigger to smaller
        pairs = []
        for i, cl in enumerate(tpc.fClusters):
            alpha = 2 if cl.GetIsBeamLike() else 1
            pairs.append((i, alpha * cl.GetSizeOfVoxels()))
        pairs = sorted(pairs, key=lambda x : x[1], reverse=True)
        ids = [e[0] for e in pairs]
        for i, cl in enumerate(tpc.fClusters):
            id = ids.index(i) if i in ids else np.nan
            for v in cl.GetVoxels():
                l = [v.GetPosition().X(), v.GetPosition().Y(), v.GetPosition().Z(), v.GetCharge(), id]
                self.voxels.append(l)
            line = cl.GetLine()
            self.lines.append(line)
        if tpc.fRPs.size() == 1:
            rp = tpc.fRPs.front()
            self.rp = [rp.X(), rp.Y(), rp.Z()]

    def fill(self, proj: str = "xy"):
        if proj not in ["xy", "xz", "d3"]:
            raise ValueError(f"Invalid value for 'what': {proj}")

        matrix = self.xy if proj == "xy" else (self.xz if proj == "xz" else self.d3)
        if matrix is not None:
            return
        isd3 = False
        if matrix is None:
            if proj == "xy" or proj == "xz":
                matrix = np.full((self.dims[0], self.dims[1], 2), np.nan)
            else:## 3D
                matrix = np.full(self.dims, np.nan) ## save only charge for 3D
                isd3 = True

        for v in self.voxels:
            x, y, z, q, id = v
            x = int(x)
            y = int(y)
            z = int(z)
            i = x
            j = y if proj == "xy" or proj == "d3" else z
            k = z
            if not isd3:
                if np.isnan(matrix[i, j, 0]):
                    matrix[i, j, 0] = q
                else:
                    matrix[i, j, 0] += q
                matrix[i, j, 1] = id
            else:
                if np.isnan(matrix[i, j, k]):
                    matrix[i, j, k] = q
                else:
                    matrix[i, j, k] += q
        # Assign back
        if proj == "xy":
            self.xy = matrix
        elif proj == "xz":
            self.xz = matrix
        elif proj == "d3":
            self.d3 = matrix
        return

    def change_id(self, vals: dict)-> None:
        for matrix in [self.xy, self.xz]:
            if matrix is None:
                continue
            for i,j in zip(*np.where(~np.isnan(matrix[:,:,1]))):
                prev = matrix[i, j, 1]
                if prev in vals.values():
                    matrix[i, j, 1] = vals[prev]
        return

    def draw(self, id: bool = False, proj: str = "xy") -> mpl.collections.QuadMesh:
        if proj not in ["xy", "xz"]:
            raise ValueError(f"Invalid projection to plot in 2D: {proj}")
        matrix = self.xy if "xy" in proj else self.xz
        if matrix is None:
            self.fill(proj)
        matrix = self.xy if "xy" in proj else self.xz
        # Get axis
        ax = plt.gca()
        mesh = ax.pcolormesh(matrix[:, :, 1 if id else 0].T, cmap=cmc.managua_r, edgecolors="none", linewidth=0, rasterized=True)
        # Nice formatting
        ax.set_xlabel("X [pads]", loc="right")
        ax.set_xlim(0, 128)
        ax.set_ylim(0, 128)
        ticks = np.arange(0, 128, 40)
        ax.set(xticks=ticks, yticks=ticks)
        if "xy" in proj:
            ax.set_ylabel("Y [pads]", loc="top")
        else:
            ax.set_ylabel("Z [time buckets]", loc="top")
        return mesh

    def draw_3d(self) -> mpl.collections.PathCollection:
        if self.d3 is None:
            self.fill("d3")
        # Mask NaNs
        ok = ~np.isnan(self.d3)
        x, y, z = np.where(ok)
        q = self.d3[x, y, z]
        x = x + 0.5
        y = y + 0.5
        z = z + 0.5
        # Get axis
        ax = plt.gca()
        scat = ax.scatter(x, y, z, c=q, cmap=cmc.managua_r, marker="o", s=25, edgecolor="none", linewidth=0.3, alpha=0.85)
        pad = 10
        ax.set_xlabel("X [pads]", labelpad=pad)
        ax.set_ylabel("Y [pads]", labelpad=pad)
        ax.set_zlabel("Z [time bucket]", labelpad=pad)
        labels = np.arange(0, 128, 40)
        ax.set(xticks=labels, yticks=labels, zticks=labels)
        ax.set_zlim(0, 128)
        ax.set_ylim(0, 128)
        ax.set_xlim(0, 128)
        return scat
    
    def draw_line(self, idx: int, min: float = 0, max: float = 128, **kwargs)-> None:
        vx = [min, max]
        vy = []
        for x in vx:
            p = self.lines[idx].MoveToX(x)
            vy.append(p.Y())
        plt.gca().plot(vx, vy, **kwargs)

    def draw_rp(self, **kwargs) -> None:
        if np.any(self.rp == np.nan):
            return
        plt.gca().plot(self.rp[0], self.rp[1], marker="*", **kwargs)

    def eval_line(self, idx: int, x: float) -> float:
        if idx < len(self.lines):
            p = self.lines[idx].MoveToX(x)
            return p.Y()
        return -11
    
def add_subplot_label(label:str, x: float = 0.925, y: float = 0.910)-> None:
    ax = plt.gca()
    ax.text(x, y, rf"\textbf{{{label}}}", transform=ax.transAxes, fontsize=18, va="center", ha="center") 

def init_figure(name: str, **kwargs)-> plt.Figure:
    fig = plt.figure(name, **kwargs)
    fig.clf()
    return fig

"""
Annotation default style
"""
ann_style: dict = {
    "fontsize": 12,
    "arrowprops": {"arrowstyle": "-"}
}