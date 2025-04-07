import os
import re
import pyphysics as phys
import numpy as np
import ROOT as r
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import uncertainties as un
import uncertainties.unumpy as unp
from cycler import cycler

path = "/media/Data/E796v2/Fits/pp/Search/"
state = "g1_BG"
part = phys.Particle(8, 20)


def read_beta_search(path) -> dict:
    ret = {}
    for dir in os.scandir(path):
        if not dir.is_dir():
            continue
        fort = os.path.join(dir.path, "fort.202")
        if os.path.isfile(fort):
            match = re.search(r"[-+]?\d*\.\d+", dir.name)
            beta = float(match.group()) if match else None
            if beta is not None:
                ret[beta] = fort

    return dict(sorted(ret.items()))


# Read betas
theos = read_beta_search(os.path.join(path, state))


def parse_tgraph(g) -> np.ndarray:
    x = []
    y = []
    ey = []
    for i in range(g.GetN()):
        x.append(g.GetPointX(i))
        y.append(g.GetPointY(i))
        ey.append(g.GetErrorY(i))
    return np.column_stack((x, y, ey))


# Read experimental xs
with r.TFile("./Outputs/psamp.root") as f:  # type: ignore
    factors = f.Get("Factors")
    mg = f.Get("mgFirst")
    exp = {}
    for f, g in zip(factors, mg.GetListOfGraphs()):
        exp[f] = parse_tgraph(g)


# Beta search
def beta_search(exp: np.ndarray, betas: dict, show: bool = False):
    # Init comparator
    comp = phys.Comparator(exp)
    # Add models
    for b, file in betas.items():
        comp.add_model(str(b), file)
    comp.fit(show)
    x, y = zip(*[(float(key), sf) for key, sf in comp.fSFs.items()])
    return np.column_stack((x, y))


lines = {}
for f, data in exp.items():
    res = beta_search(data, theos)
    lines[f] = res

# EM beta
embeta = un.ufloat(5.9, 0.2)
embeta = phys.BE_to_beta(embeta, part, 2, False)


# Now find betas and MnMps
def find_betas(lines: dict, ref: float | un.UFloat) -> tuple:
    retbeta = {}
    retfound = {}
    for f, d in lines.items():
        # Build spline
        spe = phys.create_spline3(d[:, 0], unp.nominal_values(d[:, 1]))
        # Find root
        min = d[:, 0].min()
        max = d[:, 0].max()
        nuclear = 0
        try:
            nuclear = phys.find_root(spe, 1, [min, max])
        except ValueError:
            print(
                "Assert the theoretical beta range is large enough so we have a y = 1 crossing point"
            )
        # And compute scaling factor needed for reference beta
        found = spe(unp.nominal_values(ref))
        retbeta[f] = nuclear
        retfound[f] = found
    return (retbeta, retfound)


# E. Khan's beta
khan = un.ufloat(0.55, 0.06)
beta, needed_sf = find_betas(lines, khan)
# E. Khan's data
khan_xs = phys.parse_txt("../Reanalysis/inelastic.dat")
# Print to terminal
for f, sf in needed_sf.items():
    print(f"Factor {f} needs scaling : {sf:.2f}")
fig, axs = plt.subplots(2, 2, figsize=(8, 6))
# Read data
ax: mplaxes.Axes = axs[0][0]
for f, d in exp.items():
    ax.errorbar(d[:, 0], d[:, 1], yerr=d[:, 2], marker="s", label=f"f = {f}")
ax.plot(khan_xs[:, 0], khan_xs[:, 1], marker="o", label="E. Khan")
ax.set_xlabel(r"$\theta_{\mathrm{CM}} [^{\circ}]$")
ax.set_ylabel(r"xs [mb/sr]")
ax.legend()

# Interpolation curves
ax: mplaxes.Axes = axs[0][1]
for f, d in lines.items():
    ax.errorbar(
        d[:, 0],
        unp.nominal_values(d[:, 1]),
        yerr=unp.std_devs(d[:, 1]),
        marker="s",
        label=f"f = {f}",
    )
ax.legend()
ax.set_xlabel(r"$\beta$")
ax.set_ylabel(r"Scaling factor")

## Search results
ax: mplaxes.Axes = axs[1][0]
ax.errorbar(
    *zip(*[(f, unp.nominal_values(b)) for f, b in beta.items()]),
    yerr=[unp.std_devs(b) for b in beta.values()],
    marker="s",
)
ax.set_xlabel("d-breakup scaling")
ax.set_ylabel(r"Exp. $\beta$")
ax.axhspan(khan.n - khan.s, khan.n + khan.s, color="red", alpha=0.25, label="E. Khan")
ax.axhline(khan.n, color="crimson")  # type: ignore
ax.legend()

# SF needed
ax: mplaxes.Axes = axs[1, 1]
ax.errorbar(
    *zip(*[(f, unp.nominal_values(sf)) for f, sf in needed_sf.items()]),
    yerr=[unp.std_devs(sf) for sf in needed_sf.values()],
    marker="s",
)
ax.set_xlabel("d-breakup scaling")
ax.set_ylabel("Factor needed in xs")


fig.tight_layout()
# fig.savefig("./Pictures/psmnmp.png", dpi=200)
plt.show()
