# pyright: ignore[attr-defined]
import ROOT as r
import uncertainties as unc
from uncertainties import unumpy
import matplotlib.pyplot as plt
import matplotlib.font_manager as fm
import math

import baragerUtils

prop = fm.FontProperties(fname="/usr/share/fonts/truetype/msttcorefonts/times.ttf")
plt.rcParams.update(
    {
        "text.usetex": False,
        "font.family": prop.get_name(),
        # "font.serif": ["Times"],
        "axes.titlesize": 18,
        "axes.labelsize": 14,
        "axes.prop_cycle": plt.cycler(color=plt.get_cmap("Set2").colors),
        "xtick.labelsize": 12,
        "ytick.labelsize": 12,
        "legend.fontsize": 12,
    }
)

o20 = r.ActPhysics.Particle("20O")  # type: ignore[attr-defined]
o21 = r.ActPhysics.Particle("21O")  # type: ignore[attr-defined]


def ApplySystematic(sf: unc.UFloat) -> unc.UFloat:
    percent = 0.3
    nominal = sf.n
    usys = nominal * percent
    sf.std_dev = math.sqrt(sf.s**2 + usys**2)


class SpeInfo:
    def __init__(self, ex: float | unc.UFloat, sf: float | unc.UFloat):
        self.fEx = ex
        self.fSF = sf

    def Ex(self) -> float | unc.UFloat:
        return self.fEx

    def SF(self) -> float | unc.UFloat:
        return self.fSF


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
    exp[state.decode("utf-8")] = SpeInfo(ex, sf)

## Map Jpis to states
map = {"5/2+": ["g0"], "1/2-": ["g2", "v0", "v1", "v4"]}

## And read adding measurements
adding = {"5/2+": [SpeInfo(unc.ufloat(0, 0), unc.ufloat(0.34, 0.03))], "1/2-": []}

## Read theoretical YSOX
pos = baragerUtils.parse("./Inputs/SM/log_O20_O19_ysox_tr_j0p_m1p.txt")
neg = baragerUtils.parse("./Inputs/SM/log_O20_O19_ysox_tr_j0p_m1n.txt")
ysox = pos + neg
baragerUtils.shift_Ex(ysox)


## And compute theoretical position of Gap
def computeTheo(n: int, l: int, J: str, theo: list, ex: float = 10) -> unc.UFloat:
    expression = "".join(c for c in J if c.isdigit() or c == "/")
    j = float(eval(expression))
    states = []
    for state in theo:
        if state.fn == n and state.fl == l and state.fj == j and state.fEx <= ex:
            states.append(state)
    a = sum(
        (2 * j + 1) * info.SF() * (info.Ex() + (-1) * o21.GetSn()) for info in adding[J]
    )
    b = sum(state.fc2s * (-1 * o20.GetSn() - state.fEx) for state in states)
    c = sum((2 * j + 1) * info.SF() for info in adding[J])
    d = sum(state.fc2s for state in states)
    # print("c : ", c, " d : ", d)
    return (a + b) / (c + d)


theod52 = computeTheo(0, 2, "5/2+", ysox)
theop12 = computeTheo(0, 1, "1/2-", ysox, 20)
theogap = abs(theod52 - theop12)


def compute(J: str, scale: float = 1) -> unc.UFloat:
    # Convert to value
    expression = "".join(c for c in J if c.isdigit() or c == "/")
    j = float(eval(expression))
    # print("j : ", j)
    a = sum(
        (2 * j + 1) * info.SF() * (info.Ex() + (-1) * o21.GetSn()) for info in adding[J]
    )
    b = sum(
        scale * info.SF() * (-1 * o20.GetSn() - info.Ex())
        for s in map[J]
        if (info := exp[s]) is not None
    )
    # print("a : ", a, " b : ", b)
    c = sum((2 * j + 1) * info.SF() for info in adding[J])
    d = sum(scale * info.SF() for s in map[J] if (info := exp[s]) is not None)
    # print("c : ", c, " d : ", d)
    return (a + b) / (c + d)


d52 = compute("5/2+")
p12 = compute("1/2-")
# Increasing SFs to (d,d) normalization
d52scaled = compute("5/2+", 1.42)
p12scaled = compute("1/2-", 1.42)

print("==== Baranger formula for 20O =====")
print("==== N = 8 =====")
print(" 1d5/2 : ", d52)
print("  theo : ", theod52)
print(" 1p1/2 : ", p12)
print("  theo : ", theop12)
print(" Gap   : ", abs(d52 - p12))

# Plot some things
fig, axes = plt.subplots(2, 2, figsize=(10, 8))
ax1, ax2, ax3, ax4 = axes.flatten()
# Number of protons
z = [6, 8, 12]
# 1: Using PROTON normalization
yd52 = [-0.47, -4.1, d52]
yp12 = [-8.2, -15.6, p12]
# Compute gap
gap = [abs(abs(a) - abs(b)) for a, b in zip(yd52, yp12)]
ax1.errorbar(
    z,
    unumpy.nominal_values(yd52),
    yerr=unumpy.std_devs(yd52),
    marker="o",
    markerfacecolor="none",
    label="$1d_{5/2}$",
)
ax1.errorbar(
    z,
    unumpy.nominal_values(yp12),
    yerr=unumpy.std_devs(yp12),
    marker="o",
    markerfacecolor="none",
    label="$1p_{1/2}$",
)
ax1.set_xlabel("Z")
ax1.set_ylabel("Binding energy [MeV]")
ax1.set_title("Proton norm.")
ax1.legend()
# 2: using DEUTON normalization
yd52scaled = [-0.47, -4.1, d52scaled]
yp12scaled = [-8.2, -15.6, p12scaled]
gapscaled = [abs(abs(a) - abs(b)) for a, b in zip(yd52scaled, yp12scaled)]
ax2.errorbar(
    z,
    unumpy.nominal_values(yd52scaled),
    yerr=unumpy.std_devs(yd52scaled),
    marker="o",
    markerfacecolor="none",
    label="$1d_{5/2}$",
)
ax2.errorbar(
    z,
    unumpy.nominal_values(yp12scaled),
    yerr=unumpy.std_devs(yp12scaled),
    marker="o",
    markerfacecolor="none",
    label="$1p_{1/2}$",
)
ax2.set_xlabel("Z")
ax2.set_ylabel("Binding energy [MeV]")
ax2.set_title("Deuton norm.")
ax2.legend()
## Gaps
ax3.errorbar(
    z,
    unumpy.nominal_values(gap),
    yerr=unumpy.std_devs(gap),
    marker="o",
    markerfacecolor="none",
)
ax3.errorbar(
    z[-1],
    [theogap.n],
    yerr=[theogap.s],
    marker="s",
    markeredgecolor="black",
    markerfacecolor="none",
)
ax3.set_xlabel("Z")
ax3.set_ylabel("Gap [MeV]")
ax3.set_title("Gap")
## Gap
ax4.errorbar(
    z,
    unumpy.nominal_values(gapscaled),
    yerr=unumpy.std_devs(gapscaled),
    marker="o",
    markerfacecolor="none",
)
ax4.errorbar(
    z[-1],
    [theogap.n],
    yerr=[theogap.s],
    marker="s",
    markeredgecolor="black",
    markerfacecolor="none",
)
ax4.set_xlabel("Z")
ax4.set_ylabel("Gap [MeV]")
ax4.set_title("Gap")
plt.tight_layout()
# Save
plt.savefig("./Pictures/Feb25/n_8_preliminary.png")
plt.show()
