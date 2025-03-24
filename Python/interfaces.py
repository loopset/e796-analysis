from math import sqrt
import ROOT as r
import uncertainties as unc
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt

from shell_model import ShellModelData, ShellModel


class ExpeState:
    """
    A class representating SF information for the same Ex state
    This includes different theoretical models being fitted to the experimental data
    """

    def __init__(self) -> None:
        self.Ex: unc.UFloat = unc.ufloat(-1, 0)
        self.Models: list = []
        self.SFs: list = []
        self.Chis: list = []

    def get_best(self) -> tuple:
        # Very important of ordeR: float has to be 1st element in tuple of list expression
        if not len(self.Chis):
            return (0, 0, 0)
        lowest, idx = min((chi, i) for i, chi in enumerate(self.Chis))
        return (self.Models[idx], self.SFs[idx], lowest)

    def __str__(self) -> str:
        return f"ExpeState Ex : {self.Ex} and {len(self.SFs)} SF models"


class FitInterface:
    def __init__(self, interface: str, fit: str, ang: str) -> None:
        self.data: dict = {}
        self.best: dict = {}

        # Interface
        inter = r.Fitters.Interface()  # type: ignore[attr-defined]
        inter.Read(interface, fit)
        # Angular fits
        file = r.TFile(ang)  # type: ignore[attr-defined]
        for key in file.Get("Keys"):
            key = key.decode("utf-8")
            # Init class
            expe = ExpeState()
            # Ex
            ex = unc.ufloat(inter.GetParameter(key, 1), inter.GetUnc(key, 1))
            expe.Ex = ex
            # Collection
            col = file.Get(f"{key}_sfs")
            # Names
            names = col.GetModels()
            # SFs
            sfs = col.GetSFs()
            # Fill expe
            for i, name in enumerate(names):
                expe.Models.append(name)
                sf = unc.ufloat(sfs[i].GetSF(), sfs[i].GetUSF())
                expe.SFs.append(sf)
                chi = sfs[i].GetChi2Red()
                expe.Chis.append(chi)
            # And push
            self.data[key] = expe

        # And get best models
        self.__get_best()
        return

    def __get_best(self) -> None:
        for key, vals in self.data.items():
            ex = vals.Ex
            _, sf, _ = vals.get_best()
            self.best[key] = ShellModelData(ex, sf)
        return

    def map(self, mappings: dict) -> ShellModel:
        sm = ShellModel()
        for q, keys in mappings.items():
            sm.data[q] = []
            for key in keys:
                if key in self.best:
                    sm.data[q].append(self.best[key])
        return sm

    def add_systematic(self, factor: float = 0.25) -> None:
        for key, expe in self.data.items():
            for sf in expe.SFs:
                systematic = sf.n * factor
                sf.std_dev = sqrt(sf.s**2 + systematic**2)
        # Also for best
        for key, data in self.best.items():
            systematic = data.SF.n * factor
            data.SF.std_dev = sqrt(data.SF.s**2 + systematic**2)
        return


class TPCDataInterface:
    def __init__(
        self, file: str, clusters: list = [], dims: tuple = (128, 128, 128)
    ) -> None:
        self.voxels = []
        self.lines = []
        self.dims = dims
        self.rp = None
        self.xy = None
        self.xz = None
        self.d3 = None

        # By default, read XY projection
        self.parse(file, clusters)
        self.fill()

        return

    def parse(self, file: str, clusters: list = []) -> None:
        f = r.TFile(file)
        tpc = f.Get("TPCData")
        # Sort clusters from bigger to smaller
        pairs = []
        for i, cl in enumerate(tpc.fClusters):
            alpha = 2 if cl.GetIsBeamLike() else 1
            pairs.append((i, alpha * cl.GetSizeOfVoxels()))
        pairs = sorted(pairs, key=lambda x: x[1], reverse=True)
        ids = [e[0] for e in pairs]
        for i, cl in enumerate(tpc.fClusters):
            if len(clusters):  ## plot only given clusters
                if i not in clusters:
                    continue
            id = ids.index(i) if i in ids else np.nan
            for v in cl.GetVoxels():
                l = [
                    v.GetPosition().X(),
                    v.GetPosition().Y(),
                    v.GetPosition().Z(),
                    v.GetCharge(),
                    id,
                ]
                self.voxels.append(l)
            line = cl.GetLine()
            self.lines.append(line)
        ## Add noise if not passed cluster
        if len(clusters) == 0:
            for v in tpc.fRaw:
                l = [
                    v.GetPosition().X(),
                    v.GetPosition().Y(),
                    v.GetPosition().Z(),
                    v.GetCharge(),
                    -11,
                ]
                self.voxels.append(l)
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
            else:  ## 3D
                matrix = np.full(self.dims, np.nan)  ## save only charge for 3D
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

    def change_id(self, vals: dict) -> None:
        for matrix in [self.xy, self.xz]:
            if matrix is None:
                continue
            for i, j in zip(*np.where(~np.isnan(matrix[:, :, 1]))):
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
        mesh = ax.pcolormesh(
            matrix[:, :, 1 if id else 0].T,
            cmap="managua_r",
            edgecolors="none",
            linewidth=0,
            rasterized=True,
        )
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
        scat = ax.scatter(
            x,
            y,
            z,
            c=q,
            cmap="managua_r",
            marker="o",
            s=25,
            edgecolor="none",
            linewidth=0.3,
            alpha=0.85,
        )
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

    def draw_line(self, idx: int, min: float = 0, max: float = 128, **kwargs) -> None:
        vx = [min, max]
        vy = []
        for x in vx:
            p = self.lines[idx].MoveToX(x)
            vy.append(p.Y())
        plt.gca().plot(vx, vy, **kwargs)

    def draw_rp(self, **kwargs) -> None:
        if np.any(self.rp == np.nan) or self.rp is None:
            return
        plt.gca().plot(self.rp[0], self.rp[1], marker="*", **kwargs)

    def eval_line(self, idx: int, x: float) -> float:
        if idx < len(self.lines):
            p = self.lines[idx].MoveToX(x)
            return p.Y()
        return -11
