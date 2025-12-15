import pyphysics as phys

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as un
import pandas as pd

from BetaFinder import BetaFinder

o20 = phys.Particle("20O")

# Set sys for calculations
BetaFinder.set_stat_unc_iter(1000)
BetaFinder.set_sys_unc(0.20)

# Read interpolation for g1
g1 = BetaFinder("../Outputs/xs/g1_xs.dat", "./g1_Daeh/l2", "202")

# For each beta2 em evaluate the bernstein formula
bem = {"Zanon": 29.5, "Toshio": 17}  ## Beta UP, in units of e2fm4
betasem = {k: phys.BE_to_beta(em, o20, 2) for k, em in bem.items()}

# Find MnMp with no correction
uncorr = phys.Bernstein(o20, g1.fBeta, betasem["Zanon"], 1)
print(
    "MnMp 2+1 uncorrected : ", phys.simple_bernstein(betasem["Zanon"], g1.fBeta, o20, 1)
)
print("MnMp 2+1 uncorr gen : ", uncorr.fMnMp)

# Evaluate Bernstein formula
betadd = np.arange(0.1, 0.6, 0.025, dtype=float)
mnmps = {}
mnmps_gen = {}
for k, em in betasem.items():
    mnmps[k] = [phys.simple_bernstein(em, nuclear, o20, 1) for nuclear in betadd]
    mnmps_gen[k] = [phys.Bernstein(o20, nuclear, em, 1).fMnMp for nuclear in betadd]

# Find intersection with E. Khan value for 2+1
# Simple formula
# ekhan = un.ufloat(2.35, 0.37) * 1.5  # type: ignore 1.5 comes from N/Z = 1.5 for 20O
ekhan = phys.simple_bernstein(betasem["Zanon"], un.ufloat(0.55, 0.06), o20, 1 / 3)
ekhan_gen = phys.Bernstein(o20, un.ufloat(0.55, 0.06), betasem["Zanon"], 1.0 / 3).fMnMp
print(f"E. Khan MnMp 2+1 : {ekhan:.2uS}")
print(f"E. Khan MnMp 2+1 gen : {ekhan_gen:.2uS}")

# Determine which value to use!
isGen = False
print(f"Which formula ? {'Simple' if not isGen else 'Generalised'}")

roots = {}
for key in mnmps.keys():
    yvalues = mnmps_gen[key] if isGen else mnmps[key]
    yvalue = ekhan_gen if isGen else ekhan
    spline = phys.create_spline3(betadd, yvalues)
    root = phys.find_root(spline, un.nominal_value(yvalue), [0, 1])
    roots[key] = root
    # print("Key : ", key, " Root : ", root)


# And find scaling factor in our xs
sfs = {}
for k, root in roots.items():
    sfs[k] = 1.0 / (g1.fSpline(root) if g1.fSpline is not None else 1)
    # print("Key : ", k, " SF : ", sfs[k])

# States
g2 = BetaFinder("../Outputs/xs/g2_xs.dat", "./g2_Daeh/l2", "202")
g3 = BetaFinder("../Outputs/xs/g3_xs.dat", "./g3_Daeh/l3", "202")
g5 = BetaFinder("../Outputs/xs/g5_xs.dat", "./g5_Daeh/l3", "202")
g6 = BetaFinder("../Outputs/xs/g6_xs.dat", "./g6_Daeh/l2", "202")
states = {
    "2+1": g1,
    "2+2": g2,
    "3-1": g3,
    "3-2": g5,
    "2+3": g6,
}

# Betas EM
states_em = {
    "2+1": betasem["Zanon"],
    "2+2": phys.BE_to_beta(un.ufloat(1.3, 0.2), o20, 2, False),  # Zanon
    "3-1": phys.BE_to_beta(un.ufloat(1.19e3, 0.10e3), o20, 3, True),  # Nakatsuka et al
    "3-2": np.nan,  # No value yet
    "2+3": np.nan,  # No value yet
}

scaling = sfs["Zanon"]
print(f"Using SF : {scaling}")
states_beta = {}
states_mnmp = {}
states_mnmp_gen = {}  # generalised Bernstein formula
for state, finder in states.items():
    finder.scale(scaling)
    states_beta[state] = finder.fBeta
    states_mnmp[state] = phys.simple_bernstein(states_em[state], finder.fBeta, o20, 1)
    b = phys.Bernstein(o20, finder.fBeta, states_em[state], 1)
    states_mnmp_gen[state] = b.fMnMp
    print("================================")
    print(f"State {state} beta  : ", states_beta[state])
    print(f"State {state} Mn/Mp : ", states_mnmp[state])
    print(f"State {state} Mn/Mp  gen: ", states_mnmp_gen[state])

fig, axs = plt.subplots(2, 2, layout="constrained")
ax: mplaxes.Axes = axs[0, 0]
for k, mnmp in mnmps.items():
    l = ax.plot(betadd, mnmp, label=k)
    ax.plot(betadd, mnmps_gen[k], color=l[0].get_color(), ls="--")

ax.axhline(
    un.nominal_value(ekhan),
    color="crimson",
    label=f"Khan : {ekhan:.2uS}",
)
ax.axhline(
    un.nominal_value(ekhan_gen),
    color="crimson",
    ls="--",
    label=f"Khan gen : {ekhan_gen:.2uS}",
)
ax.legend()
ax.set_xlabel(r"$\beta_{dd}(2^+_1)$")
ax.set_ylabel(r"$M_n/M_p$")

ax = axs[0, 1]
g1.plot(ax=ax)
ax.set_title(r"$2^+_1$ scaling")

ax = axs[1, 0]
for state, g in states.items():
    g.plot(ax, label=state)
ax.axhline(1, color="crimson")
ax.set_ylim(0, 1.5)
ax.legend()

ax = axs[1, 1]
ax.errorbar(
    [k for k in states_mnmp.keys()],
    [un.nominal_value(v) for v in states_mnmp.values()],
    yerr=[un.std_dev(v) for v in states_mnmp.values()],
    color="dodgerblue",
    marker="o",
    ms=4,
)
ax.errorbar(
    [k for k in states_mnmp_gen.keys()],
    [un.nominal_value(v) for v in states_mnmp_gen.values()],
    yerr=[un.std_dev(v) for v in states_mnmp_gen.values()],
    color="dodgerblue",
    ls="--",
    marker="o",
    ms=4,
)
# Twin axis for beta values
twin = ax.twinx()
twin.errorbar(
    [k for k in states_beta.keys()],
    [un.nominal_value(v) for v in states_beta.values()],
    yerr=[un.std_dev(v) for v in states_beta.values()],
    marker="o",
    ms=4,
    color="green",
)
twin.set_ylabel(r"$\beta$", color="green")
twin.tick_params(axis="y", labelcolor="green")


# Value of E. Khan for 3-1 using exp betapp and
# theoretical paper beta3
ekhan3 = phys.simple_bernstein(states_em["3-1"], un.ufloat(0.35, 0.05), o20, 1 / 3)
ekhan3_gen = phys.Bernstein(o20, un.ufloat(0.35, 0.05), states_em["3-1"], 1 / 3).fMnMp
print("E.Khan Mn/Mp 3-1 : ", ekhan3)
print("E.Khan Mn/Mp 3-1 gen : ", ekhan3_gen)
ax.hlines(y=un.nominal_value(ekhan), xmin=-0.25, xmax=0.25, color="crimson")
ax.hlines(
    y=un.nominal_value(ekhan_gen), xmin=-0.25, xmax=0.25, color="crimson", ls="--"
)
ax.hlines(y=un.nominal_value(ekhan3), xmin=1.75, xmax=2.25, color="crimson")
ax.hlines(
    y=un.nominal_value(ekhan3_gen), xmin=1.75, xmax=2.25, color="crimson", ls="--"
)
ax.set_xlabel("State")
ax.set_ylabel(r"$M_n/M_p$")

# Build df
df = pd.DataFrame(
    {
        "state": [key for key in states_beta.keys()],
        "beta": [beta for beta in states_beta.values()],
        "mnmp": [mnmp for mnmp in states_mnmp.values()],
        "mnmp_gen": [mnmp_gen for mnmp_gen in states_mnmp_gen.values()],
    }
)


def fmt(val):
    if hasattr(val, "n"):
        return f"{val:.2uS}"
    else:
        return val

df = df.map(fmt)  # type: ignore
print(df.T.to_latex())
# fig.tight_layout()

plt.show()
