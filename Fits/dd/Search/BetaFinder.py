import pyphysics as phys
import numpy as np
from typing import Dict, List, Any, Optional, Union
from numpy.typing import NDArray
from pathlib import Path
import uncertainties as un
import uncertainties.unumpy as unp
from scipy.interpolate import CubicSpline
import re
import matplotlib.pyplot as plt
from matplotlib.axes import Axes


class BetaFinder:
    _sys_unc: float = 0
    _stat_unc_iter: int = 100

    def __init__(self, exp_file: str, model_dir: str, fort: str, scale=1) -> None:
        self.fExp: NDArray = self._parse_exp(exp_file)
        aux = self.fExp.copy()
        aux[:, 1:3] *= scale
        self.fComp: phys.Comparator = phys.Comparator(aux)
        self.fBetas: List = []
        self.fSFs: List = []
        self.fSpline: Optional[CubicSpline] = None
        self.fBeta: Union[float, un.Variable] = 0

        self._read_fit(model_dir, fort)
        self._find_beta()
        return

    def _parse_exp(self, exp) -> NDArray:
        return phys.parse_txt(exp, 3)

    def _fill_fit_res(self) -> None:
        # Clean dicts
        self.fBetas.clear()
        self.fSFs.clear()
        self.fSpline = None
        self.fBeta = 0
        # Fill results
        for k, sf in self.fComp.fSFs.items():
            self.fBetas.append(float(k))
            if self.get_sys_unc() > 0:
                sys = un.nominal_value(sf) * self.get_sys_unc()
                sf += un.ufloat(0, sys, tag="sys")
            self.fSFs.append(sf)
        return

    def _read_fit(self, model_dir, fort) -> None:
        base = Path(model_dir)
        aux: Dict[float, str] = {}
        for d in base.iterdir():
            if d.is_dir() and d.name.startswith("beta_"):
                m = re.match(r"beta_([0-9.]+)", d.name)
                if not m:
                    continue
                beta = float(m.group(1))
                file = d / f"fort.{fort}"
                if not file.exists():
                    raise ValueError(f"no fort file for beta {beta}")
                aux[beta] = str(file.resolve())
        ## Sort by ascending keys
        aux = dict(sorted(aux.items()))
        self.fComp.add_models({str(k): v for k, v in aux.items()})
        self.fComp.fit()
        # Fill results
        self._fill_fit_res()
        return

    def _find_beta(self) -> None:
        values = []
        for _ in range(self.get_stat_unc_iter()):
            sfs_it = [
                np.random.normal(loc=un.nominal_value(sf), scale=un.std_dev(sf))
                for sf in self.fSFs
            ]
            spline = phys.create_spline3(self.fBetas, sfs_it)
            value = phys.find_root(spline, 1, [min(self.fBetas), max(self.fBetas)])
            values.append(value)
        # Get mean and std_dev
        mean = np.mean(values)
        std_dev = np.std(values)
        self.fBeta = un.Variable(mean, std_dev)
        # And create default spline
        self.fSpline = phys.create_spline3(self.fBetas, unp.nominal_values(self.fSFs))

    def scale(self, scale) -> None:
        aux = self.fExp.copy()
        aux[:, 1:3] *= scale
        self.fComp.fExp = aux
        self.fComp.fFitSplines = {}
        self.fComp.fFitted = {}
        self.fComp.fit()
        # Fill results again
        self._fill_fit_res()
        # And find beta
        self._find_beta()
        return

    def plot(self, ax: Axes | None = None, title: str = "", **kwargs) -> None:
        if ax is None:
            fig, ax = plt.subplots()
        ax.errorbar(
            unp.nominal_values(self.fBetas),
            unp.nominal_values(self.fSFs),
            yerr=unp.std_devs(self.fSFs),
            **kwargs,
        )
        ax.set_xlabel(r"$\beta_{nuclear}$")
        ax.set_ylabel(r"SF")
        ax.set_title(title)

    @classmethod
    def set_sys_unc(cls, value: float) -> None:
        cls._sys_unc = value
        return

    @classmethod
    def set_stat_unc_iter(cls, value: int) -> None:
        cls._stat_unc_iter = value
        return

    @classmethod
    def get_sys_unc(cls) -> float:
        return cls._sys_unc

    @classmethod
    def get_stat_unc_iter(cls) -> int:
        return cls._stat_unc_iter
