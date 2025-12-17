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
    "v4": (rebin, qp32),
    "v5": (rebin, qp32),
    "v6": (rebin, qp32),
    "v7": (rebin, qp12),  # state at 15 MeV is 19N gs = 0p1/2
}


# Systematic uncertainties
sysNorm = 0.0325
sysOMP = 0.11
sysRWS = 0.13


def apply_systematics(x: un.Variable, withRWS: bool = True) -> un.Variable:
    sf = un.nominal_value(x)
    norm = sf * sysNorm
    omp = sf * sysOMP
    rws = 0
    if withRWS:
        rws = sf * sysRWS
    return x + un.ufloat(0, norm, "sys_norm") + un.ufloat(0, omp, "sys_omp") + un.ufloat(0, rws, "sys_rws")  # type: ignore


def parse_iter_v567() -> Dict[str, un.Variable]:
    import ROOT as r

    r.PyConfig.DisableRootLogon = True  # type: ignore
    file = r.TFile("/media/Data/E796v2/Publications/dt/Inputs/iter_v567.root")  # type: ignore
    states = ["v5", "v6", "v7"]
    ret = {}
    for state in states:
        h = file.Get(f"hSF{state}1")  # l = 1
        val = un.ufloat(h.GetMean(), h.GetStdDev(), "iter_v567")
        ret[state] = val
    return ret


def build_df(withSys: bool = True, corrOffset: bool = True) -> pd.DataFrame:
    table = {"name": [], "ex": [], "sf": [], "model": [], "chi": []}
    # Parse special SF results for v5...7
    special = ["v5", "v6", "v7"]
    iterv567 = parse_iter_v567()
    for state, (sfs, q) in assignments.items():
        ex, _ = fit.get(state)
        ex.tag = "stat_ex"  # type: ignore
        sf = next((e for e in sfs.get(state) if e.fName == equiv[q]), None)
        # Write data
        table["name"].append(state)
        table["ex"].append(ex)
        if sf is None:
            continue
        val = sf.fSF
        val.tag = "stat_sf"
        # Override for special states
        if state in special:
            val = iterv567[state]
        if withSys:
            val = apply_systematics(val)
        table["sf"].append(val)
        table["model"].append(str(sf.fName))
        table["chi"].append(sf.fChi)
    if corrOffset:
        offset, _ = fit.get("g0")
        table["ex"] = [
            ex - un.ufloat(un.nominal_value(offset), un.std_dev(offset), "sys_offset")
            for ex in table["ex"]
        ]
        # Bugfix: g0 systematic error due to offset is null! bc we're substracting the same variable
        # so they're correlated
        table["ex"][0] = un.ufloat(0, un.std_dev(offset), "stat_ex")
    return pd.DataFrame(table)


def build_sm(withSys: bool = True, corrOffset: bool = True) -> phys.SMDataDict:
    ret = defaultdict(list)
    # Create dataframe
    df = build_df(withSys, corrOffset)
    for i, row in df.iterrows():
        ex = row["ex"]
        sf = row["sf"]
        data = phys.ShellModelData(ex, sf)
        tup = assignments.get(row["name"], None)
        if tup is None:
            raise ValueError(f"Cannot locate name col in df for {row['name']} state")
        ret[tup[1]].append(data)
    return ret


def split_isospin(df: phys.SMDataDict) -> Tuple[phys.SMDataDict, phys.SMDataDict]:
    t32 = defaultdict(list)
    t52 = defaultdict(list)
    for k, vals in df.items():
        for val in vals:
            if un.nominal_value(val.Ex) < 15:  # First analogous state appears at 15 MeV
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
        ret[q] = 0
        num = 0
        den = 0
        for val in vals:
            num += (2 * q.j + 1) * val.SF * (val.Ex - zero)  # type: ignore
            den += (2 * q.j + 1) * val.SF  # type: ignore
        try:
            ret[q] = num / den
        except ZeroDivisionError:
            ret[q] = 0
    return ret


def plot_bars(
    models: List[phys.SMDataDict],
    labels: List[str],
    ax: Axes,
    first_call: bool = True,
    width: float = 0.6,
    height: float = 0.25,
    **kwargs,
) -> list:
    nmodels = len(models)
    left_padding = 0.075
    right_padding = 0.075
    texts = []
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
                tl = ax.annotate(
                    f"{sf:.2f}",
                    xy=(left - left_padding, ex),
                    ha="center",
                    va="center",
                    fontsize=10,
                )
                ## Annotate Jpi
                pi = "+" if q.l != 1 else "-"
                tr = ax.annotate(
                    f"${q.get_j_fraction()}^{{{pi}}}_{{{j + 1}}}$",
                    xy=(left + width + right_padding, ex),
                    ha="center",
                    va="center",
                    fontsize=10,
                )
                texts.append(tl)
                texts.append(tr)
    # Some axis settings
    ax.set_xticks([i + 0.5 for i in range(nmodels)], labels)
    ax.set_xlim(0, nmodels)
    ax.tick_params(axis="x", which="both", bottom=False, top=False, pad=10)
    ax.tick_params(axis="y", which="both", right=False)
    for spine in ["bottom", "top", "right"]:
        ax.spines[spine].set_visible(False)
    return texts


def print_uncs(x: un.Variable):
    print(f"Variable: {x:.2uS}")
    for v, u in x.error_components().items():
        print(f"  -> {v.tag} : {u:.6f}")
