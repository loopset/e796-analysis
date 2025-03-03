import ROOT as r
import uncertainties as unc

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
        idx, lowest = min((i, chi) for i, chi in enumerate(self.Chis))
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
