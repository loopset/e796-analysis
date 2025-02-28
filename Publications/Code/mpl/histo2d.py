import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import ROOT as r


class Histo2D:
    def __init__(self, file: str, name: str):
        self.ptr = None
        self.xedges = None
        self.yedges = None
        self.contents = None
        self.nbinsx = None
        self.nbinsy = None

        # Open file
        self.__openTFile(file, name)
        # Fill bin data
        self.__fillData()

    def __openTFile(self, file: str, name: str) -> None:
        with r.TFile(file) as f:
            self.ptr = f.Get(name)
            if self.ptr is None:
                raise Exception("cannot open ", name, " in file ", file)
            self.ptr.SetDirectory(r.nullptr)
            self.nbinsx = self.ptr.GetNbinsX()
            self.nbinsy = self.ptr.GetNbinsY()
        return

    def __fillData(self) -> None:
        self.xedges = np.array(
            [self.ptr.GetXaxis().GetBinLowEdge(i) for i in range(1, self.nbinsx + 2)]
        )
        self.yedges = np.array(
            [self.ptr.GetYaxis().GetBinLowEdge(i) for i in range(1, self.nbinsy + 2)]
        )
        self.contents = np.array(
            [
                [
                    np.nan
                    if (content := self.ptr.GetBinContent(i, j)) <= 0
                    else content
                    for j in range(1, self.nbinsy + 1)
                ]
                for i in range(1, self.nbinsx + 1)
            ]
        )

    def draw(self, xtitle: str = "", ytitle: str = "") -> mpl.collections.QuadMesh:
        ax = plt.gca()
        mesh = ax.pcolormesh(self.xedges, self.yedges, self.contents.T, shading="auto", rasterized=True)
        ax.set_xlabel(xtitle, loc="right")
        ax.set_ylabel(ytitle, loc="top")
        return mesh
