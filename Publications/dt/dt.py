from collections import defaultdict
import math
from typing import Dict, List, Tuple
import pyphysics as phys
from pyphysics.actroot_interface import FitInterface, SFInterface
import pandas as pd
import uncertainties as un

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
