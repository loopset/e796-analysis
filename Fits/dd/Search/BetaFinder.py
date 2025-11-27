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
    def __init__(self, exp_file: str, model_dir: str, fort: str, scale=1) -> None:
        self.fExp: NDArray = self._parse_exp(exp_file)
        aux = self.fExp.copy()
        aux[:, 1:3] *= scale
        self.fComp: phys.Comparator = phys.Comparator(aux)
        self.fBetas: List = []
        self.fSFs: List = []
        self.fSpline: Optional[CubicSpline] = None
        self.fBeta: Union[float, un.Variable] = 0

        self._parse_dir(model_dir, fort)
        self._find_beta()
        return

    def _parse_exp(self, exp) -> NDArray:
        return phys.parse_txt(exp, 3)

    def _parse_dir(self, model_dir, fort) -> None:
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
        for k, sf in self.fComp.fSFs.items():
            self.fBetas.append(float(k))
            self.fSFs.append(sf)
        return

    def _find_beta(self) -> None:
        self.fSpline = phys.create_spline3(self.fBetas, unp.nominal_values(self.fSFs))
        self.fBeta = phys.find_root(
            self.fSpline, 1, [min(self.fBetas), max(self.fBetas)]
        )

    def scale(self, scale) -> None:
        aux = self.fExp.copy()
        aux[:, 1:3] *= scale
        self.fComp.fExp = aux
        self.fComp.fFitSplines = {}
        self.fComp.fFitted = {}
        self.fComp.fit()
        # Clean dicts
        self.fBetas.clear()
        self.fSFs.clear()
        self.fSpline = None
        self.fBeta = 0
        # Fill results
        for k, sf in self.fComp.fSFs.items():
            self.fBetas.append(float(k))
            self.fSFs.append(sf)
        # And execute again
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
