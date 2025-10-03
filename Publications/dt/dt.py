from collections import defaultdict
import math
from typing import Dict, List, Tuple, Union
import pyphysics as phys
from pyphysics.actroot_interface import FitInterface, SFInterface
import pandas as pd
import uncertainties as un
from matplotlib.axes import Axes

import sys

sys.path.append("../")
import styling as sty

fit = FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
unrebin = SFInterface("../../Fits/dt/Outputs/sfs.root")
rebin = SFInterface("../../Fits/dt/Outputs/rebin_sfs.root")

qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
qd52 = phys.QuantumNumbers(0, 2, 2.5)
qs12 = phys.QuantumNumbers(1, 0, 0.5)

equiv = {
    qp12: "l = 1",
    qp32: "l = 1",
    qd52: "l = 2",
    qs12: "l = 0",
}

assignments = {
    "g0": (unrebin, qd52),
    "g1": (unrebin, qs12),
    "g2": (unrebin, qp12),
    "v0": (unrebin, qp12),
    "v1": (rebin, qp32),
    "v2": (rebin, qp32),
    "v3": (rebin, qp32),
    "v4": (rebin, qp12),  # start of T=5/2 states
    "v5": (rebin, qp32),
    "v6": (rebin, qp32),
    "v7": (rebin, qp12),  # state at 15 MeV. Next are background
}

# Systematic uncertainty percent
percentSys = 0.2


def build_df(withSys=False) -> pd.DataFrame:
    table = {"name": [], "ex": [], "sf": [], "model": [], "chi": []}
    for state, (sfs, q) in assignments.items():
        ex, sigma = fit.get(state)
        sf = next((e for e in sfs.get(state) if e.fName == equiv[q]), None)
        # Write data
        table["name"].append(state)
        table["ex"].append(ex)
        if sf is None:
            continue
        val = sf.fSF
        if withSys:
            unc = math.sqrt(val.s**2 + (percentSys * val.n) ** 2)
            val.std_dev = unc  # type: ignore
        table["sf"].append(val)
        table["model"].append(str(sf.fName))
        table["chi"].append(sf.fChi)
    return pd.DataFrame(table)


def build_sm(withSys=False) -> Dict[phys.QuantumNumbers, List[phys.ShellModelData]]:
    ret = defaultdict(list)
    for state, (sfs, q) in assignments.items():
        ex, sigma = fit.get(state)
        sf = next((e for e in sfs.get(state) if e.fName == equiv[q]), None)
        if sf is None:
            continue
        val = sf.fSF
        if withSys:
            unc = math.sqrt(val.s**2 + (percentSys * val.n) ** 2)
            val.std_dev = unc  # type: ignore
        data = phys.ShellModelData(ex, val)
        ret[q].append(data)
    return ret


def split_isospin(df: phys.SMDataDict) -> Tuple[phys.SMDataDict, phys.SMDataDict]:
    t32 = defaultdict(list)
    t52 = defaultdict(list)
    for k, vals in df.items():
        for val in vals:
            if un.nominal_value(val.Ex) < 10:
                t32[k].append(val)
            else:
                t52[k].append(val)
    return (t32, t52)


# Strengths
def get_strengths(
    data: phys.SMDataDict,
) -> Dict[phys.QuantumNumbers, Union[float, un.UFloat]]:
    ret = {}
    for q, vals in data.items():
        ret[q] = sum(val.SF for val in data[q])  # type: ignore
    return ret


# Centroids
def get_centroids(
    data: phys.SMDataDict,
) -> Dict[phys.QuantumNumbers, Union[float, un.UFloat]]:
    zero = 0
    ret = {}
    for q, vals in data.items():
        num = 0
        den = 0
        for val in vals:
            num += (2 * q.j + 1) * val.SF * (val.Ex - zero)  # type: ignore
            den += (2 * q.j + 1) * val.SF  # type: ignore
        ret[q] = num / den
    return ret


def plot_bars(
    models: List[phys.SMDataDict],
    labels: List[str],
    ax: Axes,
    first_call: bool = True,
    width: float = 0.6,
    height: float = 0.25,
    **kwargs,
) -> None:
    nmodels = len(models)
    left_padding = 0.075
    right_padding = 0.075

    for i, data in enumerate(models):
        for q, vals in data.items():
            if q == phys.QuantumNumbers.from_str("0d3/2"):
                continue
            for j, val in enumerate(vals):
                ex = un.nominal_value(val.Ex)
                sf = un.nominal_value(val.SF)
                max_sf = q.degeneracy()
                ## Left position of barh
                left = (i + 0.5) - width / 2
                color = sty.barplot.get(q, {}).get("ec")
                ec = color if "hatch" in kwargs else "none"
                label = None
                if first_call and (i == 0 and j == 0):
                    label = q.format()
                ## background bar
                ax.barh(
                    ex,
                    left=(i + 0.5) - width / 2,
                    width=width,
                    height=height,
                    color=color,
                    edgecolor=ec,
                    alpha=0.35,
                    **kwargs,
                )
                ## foreground bar
                ratio = sf / max_sf
                ax.barh(
                    ex,
                    left=(i + 0.5) - width / 2,
                    width=ratio * width,
                    height=height,
                    color=color,
                    edgecolor=ec,
                    alpha=0.75,
                    label=label,
                    **kwargs,
                )
                ## Annotate C2S
                ax.annotate(
                    f"{sf:.2f}",
                    xy=(left - left_padding, ex),
                    ha="center",
                    va="center",
                    fontsize=10,
                )
                ## Annotate Jpi
                pi = "+" if q.l != 1 else "-"
                ax.annotate(
                    f"${q.get_j_fraction()}^{{{pi}}}_{{{j}}}$",
                    xy=(left + width + right_padding, ex),
                    ha="center",
                    va="center",
                    fontsize=10,
                )
    # Some axis settings
    ax.set_xticks([i + 0.5 for i in range(nmodels)], labels)
    ax.set_xlim(0, nmodels)
    ax.tick_params(axis="x", which="both", bottom=False, top=False, pad=15)
    ax.tick_params(axis="y", which="both", right=False)
    for spine in ["bottom", "top", "right"]:
        ax.spines[spine].set_visible(False)
    return None
