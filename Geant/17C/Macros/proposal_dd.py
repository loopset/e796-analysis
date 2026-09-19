import pyphysics as phys
import matplotlib.pyplot as plt
import numpy as np
import uncertainties as un
import hist
import uproot

# Styles

plt.rcParams["axes.labelsize"] = 16
plt.rcParams["xtick.labelsize"] = 14
plt.rcParams["ytick.labelsize"] = 14

base2d = {
    "flow": "none",
    "cmin": 1,
    "cmap": "managua_r",
    "rasterized": True,
    "cbarpad": 0.05,
    "cbarsize": 0.2,
    "cbarextend": False,
}

ann = {"ha": "center", "va": "center", "fontsize": 14}

arrowprops = {"arrowstyle": "->", "lw": 0.75}

errorbar_line = {"ls": "-", "marker": "o", "ms": 4, "capsize": 3}
errorbar = {"ls": "none", "marker": "s", "capsize": 3}
errorbar_nols = {"marker": "s", "capsize": 3}

base1d = {
    "histtype": "step",
    "yerr": False,
    "flow": "none",
    # "capsize": 0,
    "lw": 1,
    # "marker": "none",
}

file = uproot.open("../Outputs/yield_17C_d_d_255.0.root")

hEx = file.get("hExAll").to_hist()  # type: ignore
hKin = file.get("hKinAll").to_hist()  # type: ignore
gRes = file.get("gRes")
# read all states
hExs = []
for key in file.keys():
    if key.startswith("hEx") and "All" not in key:
        hExs.append(file.get(key).to_hist())  # type: ignore
exs = [0, 0.217, 0.332, 2.15]


# Plot with kinematics, ex and resolution
fig, axs = plt.subplots(1, 2, figsize=(9, 4))
# Kinematics
ax = axs[0]
ret = hKin.plot(ax=ax, **base2d)
ret[1].set_ticks([])
# Theo kin
for ex in exs:
    theo = phys.Kinematics(f"17C(d,d)@255|{ex}").get_line3()
    label = f"{ex:.1f} MeV" if ex > 0 else "g.s."
    ax.plot(theo[0], theo[1], lw=1, ls="-", label=label)

# L1 region
l1 = 2.07
ax.axhline(l1, lw=1, ls="--", color="gray")
ax.axhspan(0, l1, color="gray", alpha=0.2)

ax.legend(fontsize=12)

ax.set_xlim(30, 90)
ax.set_ylim(0, 25)
ax.set_xlabel(r"$\theta_{lab}$ [$\circ$]")
ax.set_ylabel(r"$E_{lab}$ [MeV]")

# Ex
ax = axs[1]
for h in hExs:
    h.plot(ax=ax, **base1d)

hEx.plot(ax=ax, **{**base1d, "lw": 1, "color": "black"})

ax.set_xlim(-5, 10)
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel("Counts / 150 keV")

# Sn and S2n
c17 = phys.Particle("17C")
ax.axvline(
    c17.get_sn(),
    lw=1,
    ls="--",
    color="crimson",
    label=rf"$S_n$ = {c17.get_sn():.1f} MeV",
)
ax.axvline(
    c17.get_s2n(),
    lw=1,
    ls=":",
    color="crimson",
    label=rf"$S_{{2n}}$ = {c17.get_s2n():.1f} MeV",
)
ax.legend(fontsize=12, loc="center right")

# Inset for resolution
# axins = ax.inset_axes([0.55, 0.675, 0.4, 0.275])
# axins.errorbar(gRes.member("fX"), gRes.member("fY"), yerr=gRes.member("fEY"), **errorbar_line, color="black")  # type: ignore
# axins.set_ylim(0.3, 0.5)
# axins.set_ylabel(r"$\sigma$ [MeV]")

fig.tight_layout()
fig.savefig("./Outputs/dd_kin_ex_res.png", dpi=300)

plt.show()
