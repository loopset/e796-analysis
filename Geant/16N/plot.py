import hist
import matplotlib.pyplot as plt
import numpy as np
import pyphysics as phys
import uncertainties as un
import uncertainties.unumpy as unp
import uproot

plt.rcParams["axes.labelsize"] = 14
plt.rcParams["xtick.labelsize"] = 12
plt.rcParams["ytick.labelsize"] = 12
plt.rcParams["legend.fontsize"] = 12

file = uproot.open("./Outputs/yield_16N_d_3He_640.0.root")
hKin: hist.Hist = file.get("hKinAll").to_hist()  # type: ignore
hEx: hist.Hist = file.get("hExAll").to_hist()  # type: ignore
gRes = file.get("gRes")  # type: ignore
assert gRes is not None, "gRes is None"
gCounts = file.get("gCounts")  # type: ignore
assert gCounts is not None, "gCounts is None"

exs = [0, 0.74, 3.10, 11.0]
hsEx, effs, theoxs = [], [], []
for i in range(len(exs)):
    h = file.get(f"hEx{i}")
    if h is not None:
        hsEx.append(h.to_hist())
    g = file.get(f"eff{i}")
    if g is not None:
        effs.append(g)
    g = file.get(f"theo{i}")
    if g is not None:
        theoxs.append(g)

fig, axs = plt.subplots(2, 2, constrained_layout=True, figsize=(8, 6))
# Kinematics
ax = axs[0, 0]
hKin.plot(ax=ax, cmap="managua_r", cmin=1, flow=False, cbar=False)
# Draw theoretical line
kin = phys.Kinematics("16N(d,3He)@640|0").get_line3()
ax.plot(kin[0], kin[1], color="C0")
# Get Sn
sn15C = phys.Particle("15C").get_sn()
print(f"15C Sn : {sn15C:.3f} MeV")
# kin = phys.Kinematics(f"16N(d,3He)@640|{sn15C}").get_line3()
# ax.plot(kin[0], kin[1], color="crimson", label=r"$S_{n}$")
#
ax.set_xlabel(r"$\theta_{lab}$ [$\circ$]")
ax.set_ylabel(r"$E_{lab}$ [MeV]")
ax.set_xlim(0, 60)
ax.set_ylim(0, 70)
# Add inset with resolution
axins = ax.inset_axes([0.2, 0.5, 0.4, 0.4])
axins.errorbar(
    gRes.member("fX"),
    gRes.member("fY"),
    yerr=gRes.member("fEY"),
    ls="",
    marker="s",
    color="black",
)
axins.set_xlabel(r"$E_{x}$ [MeV]")
axins.set_xlim(-5, 15)
axins.set_ylabel(r"$\sigma$ [MeV]")
axins.set_ylim(0.45, 0.55)

# Exs
ax = axs[0, 1]
hEx.plot(ax=ax, yerr=False, flow=None, color="black")
for i, h in enumerate(hsEx):
    h.plot(ax=ax, yerr=False, flow=None, ls="-")
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel(r"Counts / 150 keV")
ax.set_xlim(-5, 15)
# Sn line
ax.axvline(
    sn15C, color="crimson", ls="--", label=r"$S_{n}(^{15}C) = $" + f"{sn15C:.2f} MeV"
)
ax.legend()

# Efficiencies
ax = axs[1, 0]
for i, g in enumerate(effs):
    x = np.linspace(0, 60, 60)
    spe = phys.create_spline3(g.values(0), g.values(1))
    ax.plot(x, spe(x), ls="-", label=rf"$E_{{x}} = {exs[i]:.1f}$")
    # ax.errorbar(
    #     g.values(0),
    #     g.values(1),
    #     yerr=g.errors("mean", 1),
    #     ls="-",
    #     label=rf"$E_{{x}} = {exs[i]:.1f}$",
    # )
ax.set_xlim(0, 60)
ax.set_ylim(0, 1)
ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
ax.set_ylabel(r"$\epsilon$")

ax.legend()

# Theo xs
ax = axs[1, 1]
for i, g in enumerate(theoxs):
    x = np.linspace(0, 60, 100)
    spe = phys.create_spline3(g.values(0), g.values(1))
    ax.plot(
        x,
        spe(x),
        ls="-",
        label=rf"$E_{{x}} = {exs[i]:.1f}$",
    )
ax.legend()
ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
ax.set_ylabel(r"$d\sigma/d\Omega$ [mb/sr]")

# New figure
fig, axs = plt.subplots(1, 3, constrained_layout=True, figsize=(7, 3.5))

# Total counts per state
# for these reference calculations (taken from Runner.cxx)
Iref = 5e3  # reference I with which calculations where made
UTref = 15  # reference UTs of exp (1 UT = 8h)
ax = axs[0]
ax.errorbar(
    gCounts.member("fX"),
    gCounts.member("fY"),
    yerr=gCounts.member("fEY"),
    ls="",
    marker="s",
    color="black",
)
ax.errorbar(
    gCounts.member("fX")[1],
    gCounts.member("fY")[1],
    # yerr=gCounts.member("fEY")[1],
    ls="",
    marker="s",
    color="none",
    mfc="dodgerblue",
    mec="none",
)
ax.set_xlim(-5, 15)
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel("Counts")
ax.annotate(
    f"I = {Iref:.1e} pps\nt = {UTref} UTs",
    xy=(0.3, 0.85),
    xycoords="axes fraction",
    ha="center",
    va="center",
    fontsize=10,
)

# Scale with beam intensity
# and take as reference 1st ex
ax = axs[1]
ref = un.ufloat(gCounts.member("fY")[1], gCounts.member("fEY")[1])
x = np.logspace(np.log10(1e2), np.log10(5e4), 20)
lambI = lambda I: (I / Iref) * ref
y = lambI(x)
ax.set_xscale("log")
ax.set_yscale("log")
ax.errorbar(
    x, unp.nominal_values(y), yerr=unp.std_devs(y), ls="", marker="s", color="black"
)
ax.set_xlim(5e1, 1e5)
ax.set_xlabel(r"I [pps]")
ax.set_ylabel(r"Counts $^{15}C_{2nd}$")
ax.annotate(
    f"t = {UTref} UTs",
    xy=(0.3, 0.85),
    xycoords="axes fraction",
    ha="center",
    va="center",
    fontsize=10,
)
ax.axvline(Iref, color="dodgerblue", ls="--")

# Scale with duration
ax = axs[2]
x = np.arange(1, 20, 1)
lambUT = lambda UT: (UT / UTref) * ref
y = lambUT(x)
ax.errorbar(
    x, unp.nominal_values(y), yerr=unp.std_devs(y), ls="", marker="s", color="black"
)
ax.set_xlabel(r"t [UT]")
ax.set_ylabel(r"Counts $^{15}C_{2nd}$")
ax.annotate(
    f"I = {Iref:.1e} pps",
    xy=(0.3, 0.85),
    xycoords="axes fraction",
    ha="center",
    va="center",
    fontsize=10,
)
ax.axvline(UTref, color="dodgerblue", ls="--")

plt.show()
