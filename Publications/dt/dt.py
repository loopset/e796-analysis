from collections import defaultdict
from typing import Dict, List
import pyphysics as phys
import pandas as pd

fit = phys.FitInterface("../../Fits/dt/Outputs/fit_juan_RPx.root")
unrebin = phys.SFInterface("../../Fits/dt/Outputs/sfs.root")
rebin = phys.SFInterface("../../Fits/dt/Outputs/rebin_sfs.root")

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
    "v7": (rebin, qp32),
}


def build_df() -> pd.DataFrame:
    table = {"name": [], "ex": [], "sf": [], "model": [], "chi": []}
    for state, (sfs, q) in assignments.items():
        ex, sigma = fit.get(state)
        sf = next((e for e in sfs.get(state) if e.fName == equiv[q]), None)
        # Write data
        table["name"].append(state)
        table["ex"].append(ex)
        if sf is None:
            continue
        table["sf"].append(sf.fSF)
        table["model"].append(str(sf.fName))
        table["chi"].append(sf.fChi)
    return pd.DataFrame(table)


def build_sm() -> Dict[phys.QuantumNumbers, List[phys.ShellModelData]]:
    ret = defaultdict(list)
    for state, (sfs, q) in assignments.items():
        ex, sigma = fit.get(state)
        sf = next((e for e in sfs.get(state) if e.fName == equiv[q]), None)
        if sf is None:
            continue
        data = phys.ShellModelData(ex, sf.fSF)
        ret[q].append(data)
    return ret
