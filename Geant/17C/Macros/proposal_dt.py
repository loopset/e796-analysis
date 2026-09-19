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

file = uproot.open("../Outputs/yield_17C_d_t_255.0.root")

hEx = file.get("hExAll").to_hist()  # type: ignore
hKin = file.get("hKinAll").to_hist()  # type: ignore
gRes = file.get("gRes")
# read all states
hExs = []
for key in file.keys():
    if key.startswith("hEx") and "All" not in key:
        hExs.append(file.get(key).to_hist())  # type: ignore
exs = [0, 1.77, 6.0]
idxs = [0, 1, 2]
effs = []
xs = []
for idx in idxs:
    effs.append(file.get(f"eff{idx}"))
    exp = file.get(f"rec{idx}")
    theo = file.get(f"theo{idx}")
    xs.append((exp, theo))
# Theoretical cross sections
theoxs = []
for l in ["s", "p", "d"]:
    data = phys.utils.parse_txt(f"../Inputs/dp/{l}.dat", ncols=3)
    theoxs.append(data)


# Plot with kinematics, ex and resolution
fig, axs = plt.subplots(1, 2, figsize=(9, 4))
# Kinematics
ax = axs[0]
ret = hKin.plot(ax=ax, **base2d)
ret[1].set_ticks([])
# Theo kin
for ex in exs:
    theo = phys.Kinematics(f"17C(d,t)@255|{ex}").get_line3()
    label = f"{ex:.1f} MeV" if ex > 0 else "g.s."
    ax.plot(theo[0], theo[1], lw=1, ls="-", label=label)

# L1 region
l1 = 2.8
ax.axhline(l1, lw=1, ls="--", color="gray")
ax.axhspan(0, l1, color="gray", alpha=0.2)

ax.legend(fontsize=12)

ax.set_xlim(0, 80)
ax.set_ylim(0, 40)
ax.set_xlabel(r"$\theta_{lab}$ [$\circ$]")
ax.set_ylabel(r"$E_{lab}$ [MeV]")

# Ex
ax = axs[1]
for h in hExs:
    h.plot(ax=ax, **base1d)

hEx.plot(ax=ax, **{**base1d, "lw": 1, "color": "black"})

ax.set_xlim(-5, 15)
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel("Counts / 150 keV")

# Sn and S2n
c18 = phys.Particle("18C")
ax.axvline(
    c18.get_sn(),
    lw=1,
    ls="--",
    color="crimson",
    label=rf"$S_n$ = {c18.get_sn():.1f} MeV",
)
ax.axvline(
    c18.get_s2n(),
    lw=1,
    ls=":",
    color="crimson",
    label=rf"$S_{{2n}}$ = {c18.get_s2n():.1f} MeV",
)
ax.legend(fontsize=12, loc="center right")

# Inset for resolution
axins = ax.inset_axes([0.55, 0.675, 0.4, 0.275])
axins.errorbar(gRes.member("fX"), gRes.member("fY"), yerr=gRes.member("fEY"), **errorbar_line, color="black")  # type: ignore
axins.set_ylim(0.2, 0.5)
axins.set_ylabel(r"$\sigma$ [MeV]")

fig.tight_layout()
fig.savefig("./Outputs/dt_kin_ex_res.png", dpi=300)

# # plt.close("all")
#####################################################################
# Effs and theo xs
fig, axs = plt.subplots(1, 2, figsize=(9, 4))
ax = axs[0]
# Efficiencies
for i, eff in enumerate(effs):
    x = eff.values(0)
    y = eff.values(1)
    n = 2
    x = np.append(x[:len(x)-len(x)%n].reshape(-1, n).mean(axis=1), x[len(x)-len(x)%n:].mean() if len(x)%n else [])
    y = np.append(y[:len(y)-len(y)%n].reshape(-1, n).mean(axis=1), y[len(y)-len(y)%n:].mean() if len(y)%n else [])
    label = fr"$E_{{x}} = ${exs[i]:.1f} MeV" if i > 0 else "g.s."
    ax.plot(x, y, label=label)

# L1 region
l1 = 17
ax.axvline(l1, lw=1, ls="--", color="gray")
ax.axvspan(0, l1, color="gray", alpha=0.1)

ax.legend(fontsize=12)
ax.set_xlim(0, 80)
ax.set_ylim(0, 1)
ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
ax.set_ylabel("Efficiency")

# # Theoretical cross sections from twofnr
# label = ["l = 0 (3.44 MeV)", "l = 1", "l = 2 (g.s., 10 MeV)"]
# colors=["crimson", "green", "dodgerblue"]
# ls=["-", ":", "--"]
# ax = axs[1]
# ax.set_yscale("log")
# for i, theo in enumerate(theoxs):
#     ax.plot(theo[:, 0], theo[:, 1], label=label[i], color=colors[i], ls=ls[i])

# # L1 region
# l1 = 17
# ax.axvline(l1, lw=1, ls="--", color="gray")
# ax.axvspan(0, l1, color="gray", alpha=0.1)

# ax.set_xlim(0, 180)
# ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
# ax.set_ylabel(r"d$\sigma$/d$\Omega$ [mb/sr]")
# ax.legend(fontsize=12)

fig.tight_layout()
fig.savefig("./Outputs/dt_eff_theoxs.png", dpi=300)

# # plt.close("all")
# ##############################################################
# # Reconstructed cross sections
# averageSF = 0.25
# label = ["g.s. l = 2", "3.44 MeV l = 0", "10 MeV l = 2"]
# colors=["dodgerblue", "crimson", "dodgerblue"]

# fig, axs = plt.subplots(1, 3, figsize=(9, 3))
# for i, (exp, theo) in enumerate(xs):
#     ax = axs[i]
#     ax.set_yscale("log")
#     ax.errorbar(exp.member("fX"), exp.member("fY"), yerr=exp.member("fEY"), **errorbar_nols, mec="black", color="black", ms=4)
#     ax.plot(theo.values(0), averageSF * theo.values(1), ls="-", color=colors[i], label=label[i])

#     # L1 region
#     l1 = 17
#     ax.axvline(l1, lw=1, ls="--", color="gray")
#     ax.axvspan(0, l1, color="gray", alpha=0.1)

#     ax.legend(fontsize=12)
#     ax.set_xlim(0, 180)
#     ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
#     ax.set_ylabel(r"d$\sigma$/d$\Omega$ [mb/sr]")

# fig.tight_layout()
# fig.savefig("./Outputs/xs_reco.png", dpi=300)

plt.show()
