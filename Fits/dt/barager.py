# pyright: ignore[attr-defined]
import ROOT as r
from matplotlib.axes import Axes
from numpy.core.shape_base import block
import uncertainties as unc
from uncertainties import unumpy
import matplotlib.pyplot as plt
import matplotlib.font_manager as fm
import math

import utils

prop = fm.FontProperties(fname="/usr/share/fonts/truetype/msttcorefonts/times.ttf")
plt.rcParams.update(
    {
        "text.usetex": False,
        "font.family": prop.get_name(),
        # "font.serif": ["Times"],
        "mathtext.fontset": "cm",
        "axes.titlesize": 20,
        "axes.labelsize": 18,
        "axes.prop_cycle": plt.cycler(color=plt.get_cmap("Set2").colors),
        "xtick.labelsize": 16,
        "ytick.labelsize": 16,
        "legend.fontsize": 16,
    }
)

o20 = r.ActPhysics.Particle("20O")  # type: ignore[attr-defined]
o21 = r.ActPhysics.Particle("21O")  # type: ignore[attr-defined]


def ApplySystematic(sf: unc.UFloat) -> unc.UFloat:
    percent = 0.3
    nominal = sf.n
    usys = nominal * percent
    sf.std_dev = math.sqrt(sf.s**2 + usys**2)


# Use interface to get Ex
inter = r.Fitters.Interface()  # type: ignore[attr-defined]
inter.Read("./Outputs/interface.root", "./Outputs/fit_juan_RPx.root")
# Open the file
file = r.TFile("./Outputs/sfs.root")  # type: ignore[attr-defined]
# Experimental info
exp = {}
for state in file.Get("Keys"):
    col = file.Get(f"{state}_sfs")
    ex = unc.ufloat(inter.GetParameter(state, 1), inter.GetUnc(state, 1))
    ## Get best chi2 model
    best = col.GetBestChi2()
    sf = unc.ufloat(best.GetSF(), best.GetUSF())
    ApplySystematic(sf)
    exp[state.decode("utf-8")] = utils.ShellModel(ex, sf)

## Build removal dataset
map = {
    utils.QuantumNumber(0, 2, 2.5): ["g0"],
    utils.QuantumNumber(0, 1, 0.5): ["g2", "v0", "v1", "v4"],
}
removal = {}
for key, sublist in map.items():
    removal[key] = []
    for name in sublist:
        removal[key].append(exp[name])

## And read adding measurements
adding = {
    utils.QuantumNumber(0, 2, 2.5): [
        utils.ShellModel(unc.ufloat(0, 0), unc.ufloat(0.34, 0.03))
    ],
    utils.QuantumNumber(0, 1, 0.5): [],
}


def barager(
    q: utils.QuantumNumber, add: dict, rem: dict, scale: float = 1, cut: float = -11
) -> utils.BaragerRes:
    # Adding
    numAdd = 0
    denomAdd = 0
    if q in add:
        for sm in add[q]:
            # Numerator
            numAdd += (2 * q.j + 1) * sm.SF * (sm.Ex - o21.GetSn())
            # Denominator
            denomAdd += (2 * q.j + 1) * sm.SF
    # Removal
    numRem = 0
    denomRem = 0
    if q in rem:
        for sm in rem[q]:
            if cut != -11:
                if sm.Ex > cut:
                    continue
            # Numerator
            numRem += scale * sm.SF * (-o20.GetSn() - sm.Ex)
            # Denominator
            denomRem += scale * sm.SF
    # And compute ESPE
    # print("dAdd : ", denomAdd, " dRem : ", denomRem)
    espe = (numAdd + numRem) / (denomAdd + denomRem)
    return utils.BaragerRes(denomAdd, denomRem, espe)


## YSOX
ysox = utils.read_theo(
    "./Inputs/SM/log_O20_O19_ysox_tr_j0p_m1p.txt",
    "./Inputs/SM/log_O20_O19_ysox_tr_j0p_m1n.txt",
)

## SFO-tls
sfotls = utils.read_theo(
    "./Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
    "./Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt",
)


def compute_gap(
    q0: utils.QuantumNumber,
    q1: utils.QuantumNumber,
    add: dict,
    rem: dict,
    scale: float = 1,
    cut: float = -11,
) -> tuple:
    # Position of first shell
    shell0 = barager(q0, add, rem, scale, cut)
    # Positon of second shell
    shell1 = barager(q1, add, rem, scale, cut)
    # Gap
    gap = abs(shell0.ESPE - shell1.ESPE)
    # Return values
    return (shell0, shell1, gap)


# Define quantum numbers
qd52 = utils.QuantumNumber(0, 2, 2.5)
qp12 = utils.QuantumNumber(0, 1, 0.5)

# Compute
gaps = []
d52Res = []
p12Res = []
for scale, rem in zip([1, 1.42, 1, 1], [removal, removal, ysox, sfotls]):
    d52, p12, gap = compute_gap(qd52, qp12, adding, rem, scale, 10)
    gaps.append(gap)
    d52Res.append(d52.RemStr)
    p12Res.append(p12.RemStr)

# Plot
labels = ["p-norm", "d-norm", "YSOX", "SFO-tls"]
fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2, figsize=(10, 5))
ax1.errorbar(
    labels,
    unumpy.nominal_values(gaps),
    yerr=unumpy.std_devs(gaps),
    fmt="o",
    markerfacecolor="none",
    markersize=10,
    capsize=5,
)
ax1.set_xlabel("Model")
ax1.set_ylabel("N = 8 gap [MeV]")

ax2.errorbar(
    labels,
    unumpy.nominal_values(d52Res),
    yerr=unumpy.std_devs(d52Res),
    fmt="o",
    label="$1d_{5/2}$",
)
ax2.errorbar(
    labels,
    unumpy.nominal_values(p12Res),
    yerr=unumpy.std_devs(p12Res),
    fmt="s",
    label="$1p_{1/2}$",
)
ax2.axhline(y=6, ls="--", lw=2, color="tab:olive", label=r"$2\cdot 5/2 + 1$")
ax2.axhline(y=2, ls="--", lw=2, color="tab:orange", label=r"$2\cdot 1/2 + 1$")
ax2.set_xlabel("Model")
ax2.set_ylabel(r"$\langle ^{20}$O $\vert ^{19}$O $\rangle$ SF strength")
ax2.legend()
plt.tight_layout()
plt.savefig("./Pictures/Feb25/n_8_preliminary.png")
plt.show(block=True)
