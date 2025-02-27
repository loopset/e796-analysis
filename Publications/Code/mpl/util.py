import ROOT as r
import numpy as np
import matplotlib.pyplot as plt
import cmcrameri.cm as cmc

class DataInterface():
    def __init__(self, file: str)-> None:
        self.voxels = []
        self.lines = []
        self.xy = np.full((128, 128, 2), np.nan)
        self.xz = np.full((128, 128, 2), np.nan)
        self.rp = np.full(3, np.nan)
        self._filledXY = False
        self._filledXZ = False
        self.parse(file)

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

    def fill(self, what: str = "xy"):
        matrix = self.xy if what == "xy" else self.xz
        alreadyFilled = self._filledXY if what == "xy" else self._filledXZ
        if alreadyFilled:
            return
        for v in self.voxels:
            x, y, z, q, id = v
            x = int(x)
            y = int(y)
            z = int(z)
            i = x
            j = y if "xy" in what else z
            if np.isnan(matrix[i, j, 0]):
                matrix[i, j, 0] = q
            else:
                matrix[i, j, 0] += q
            matrix[i, j, 1] = id
        
        # Set again
        if "xy" in what:
            self._filledXY = True
        if "xz" in what:
            self._filledXZ = True

    def imshow(self, id: bool = False, proj: str = "xy") -> None:
        matrix = self.xy if "xy" in proj else self.xz
        ax = plt.gca()
        ax.pcolormesh(matrix[:, :, 1 if id else 0].T, cmap=cmc.managua_r)
        ax.set_xlabel("X [pads]", loc="right")
        ax.set_xlim(0, 128)
        ax.set_ylim(0, 128)
        ticks = np.arange(0, 128, 40)
        ax.set(xticks=ticks, yticks=ticks)
        if "xy" in proj:
            ax.set_ylabel("Y [pads]", loc="top")
        else:
            ax.set_ylabel("Z [time buckets]", loc="top")
    
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
    
def add_subplot_label(label:str, x: float = 0.925, y: float = 0.910)-> None:
    ax = plt.gca()
    ax.text(x, y, rf"\textbf{{{label}}}", transform=ax.transAxes, fontsize=18, va="center", ha="center") 