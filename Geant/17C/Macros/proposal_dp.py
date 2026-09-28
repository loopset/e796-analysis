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

# Read file
file = uproot.open("../Outputs/yield_17C_d_p_255.0.root")

hEx = file.get("hExAll").to_hist()  # type: ignore
hKin = file.get("hKinAll").to_hist()  # type: ignore
gRes = file.get("gRes")
# read all states
hExs = []
for key in file.keys():
    if key.startswith("hEx") and "All" not in key:
        hExs.append(file.get(key).to_hist())  # type: ignore
# All exs
all_exs = [0, 1.59, 2.50, 5.20, 6.43, 10, 15]

# Selected ex for eff and xs
exs = [0, 2.50, 5.20, 10.0]
idxs = [0, 2, 3, 5]
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

# Comparators
comps = []
labels = ["l = 0", "l = 1", "l = 2"]
for i, (exp, theo) in enumerate(xs):
    arr = np.array([exp.member("fX"), exp.member("fY"), exp.member("fEY")]).T
    comp = phys.Comparator(arr)
    for j, model in enumerate(theoxs):
        comp.add_model(labels[j], "", model)
    comp.fit()
    comps.append(comp)

####################################################################################
# Plot with kinematics, ex and resolution
fig, axs = plt.subplots(1, 2, figsize=(9, 4))
# Kinematics
ax = axs[0]
ret = hKin.plot(ax=ax, **base2d)
ret[1].set_ticks([])
# Theo kin
for ex in all_exs:
    theo = phys.Kinematics(f"17C(d,p)@255|{ex}").get_line3()
    label = f"{ex:.1f}" if ex > 0 else "g.s."
    ax.plot(theo[0], theo[1], lw=1, ls="-", label=label)

# L1 region
l1 = 2.07
ax.axhline(l1, lw=1, ls="--", color="gray")
ax.axhspan(0, l1, color="gray", alpha=0.2)

ax.legend(fontsize=12, title=r"$E_{x}$ [MeV]", title_fontsize=12)

ax.set_xlim(30, 180)
ax.set_ylim(0, 25)
ax.set_xlabel(r"$\theta_{lab}$ [$\circ$]")
ax.set_ylabel(r"$E_{lab}$ [MeV]")

# Ex
ax = axs[1]
for h in hExs:
    h.plot(ax=ax, **base1d)

hEx.plot(ax=ax, **{**base1d, "lw": 1, "color": "black"})

ax.set_xlim(-5, 20)
ax.set_xlabel(r"$E_{x}$ [MeV]")
ax.set_ylabel("Counts / 150 keV")

# Sn and S2n
c18 = phys.Particle("18C")
ax.axvline(
    c18.get_sn(),
    lw=1,
    ls="--",
    color="crimson",
    label=rf"1n : {c18.get_sn():.1f}",
)
ax.axvline(
    c18.get_s2n(),
    lw=1,
    ls=":",
    color="crimson",
    label=rf"2n : {c18.get_s2n():.1f}",
)
ax.legend(fontsize=12, loc="center left", handlelength=1.5, handletextpad=0.5, borderpad=0.5, title="S [MeV]", title_fontsize=12)

# Inset for resolution
axins = ax.inset_axes([0.225, 0.675, 0.4, 0.275])
yerr = gRes.member("fEY") * 0 #type: ignore
axins.errorbar(gRes.member("fX"), gRes.member("fY"), yerr=yerr, **errorbar_line, color="black")  # type: ignore
axins.set_ylim(0.1, 0.4)
axins.set_ylabel(r"$\sigma$ [MeV]")

fig.tight_layout()
fig.savefig("./Outputs/kin_ex_res.png", dpi=300)
fig.savefig("./Outputs/kin_ex_res.pdf", dpi=300)

# plt.close("all")
#####################################################################
# Effs and theo xs
fig, axs = plt.subplots(1, 2, figsize=(9, 4))
ax = axs[0]
# Efficiencies
for i, eff in enumerate(effs):
    x = eff.values(0)
    y = eff.values(1)
    n = 2
    x = np.append(
        x[: len(x) - len(x) % n].reshape(-1, n).mean(axis=1),
        x[len(x) - len(x) % n :].mean() if len(x) % n else [],
    )
    y = np.append(
        y[: len(y) - len(y) % n].reshape(-1, n).mean(axis=1),
        y[len(y) - len(y) % n :].mean() if len(y) % n else [],
    )
    label = rf"$E_{{x}} = ${exs[i]:.1f} MeV" if i > 0 else "g.s."
    ax.plot(x, y, label=label)

# L1 region
l1 = 17
ax.axvline(l1, lw=1, ls="--", color="gray")
ax.axvspan(0, l1, color="gray", alpha=0.1)

ax.legend(fontsize=12)
ax.set_xlim(0, 100)
ax.set_ylim(0, 1)
ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
ax.set_ylabel("Efficiency")

# Theoretical cross sections from twofnr
label = ["l = 0", "l = 1", "l = 2"]
colors = ["crimson", "green", "dodgerblue"]
ls = ["-", ":", "--"]
ax = axs[1]
ax.set_yscale("log")
for i, theo in enumerate(theoxs):
    ax.plot(theo[:, 0], theo[:, 1], label=label[i], color=colors[i], ls=ls[i])

# L1 region
l1 = 17
ax.axvline(l1, lw=1, ls="--", color="gray")
ax.axvspan(0, l1, color="gray", alpha=0.1)

ax.set_xlim(0, 180)
ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
ax.set_ylabel(r"d$\sigma$/d$\Omega$ [mb/sr]")
ax.legend(fontsize=12, title="ADWA", title_fontsize=12)

fig.tight_layout()
fig.savefig("./Outputs/eff_theoxs.png", dpi=300)
fig.savefig("./Outputs/eff_theoxs.pdf", dpi=300)
# plt.close("all")

#########################################################################################################
# Reconstructed cross sections
averageSF = 0.25
titles = [r"g.s. $0^+_1$ l = 2", r"1.6 MeV $2^+_2$ l = 0", r"5.2 MeV $1^-$ l = 1", "10 MeV l = 2"]

fig, axs = plt.subplots(1, 4, figsize=(9, 3), sharey=True, constrained_layout=True)
for i, (exp, theo) in enumerate(xs):
    ax = axs[i]
    ax.set_yscale("log")
    ax.errorbar(
        exp.member("fX"),
        exp.member("fY"),
        yerr=exp.member("fEY"),
        **errorbar_nols,
        mec="black",
        color="black",
        ls="none",
        ms=4,
    )
    ## Plot fitted from comparator
    for j, fit in enumerate(comps[i].fFitted.values()):
        x = np.linspace(fit[:, 0].min(), fit[:, 0].max(), 200)
        spe = phys.utils.create_spline3(fit[:, 0], fit[:, 1])
        y = spe(x)
        ax.plot(x, y, ls=ls[j], color=colors[j], label=f"l = {j}" if i == 0 else None)
    # ax.plot(
    #     theo.values(0),
    #     averageSF * theo.values(1),
    #     ls="-",
    #     color=colors[i],
    #     label=label[i],
    # )

    # L1 region
    l1 = 17
    ax.axvline(l1, lw=1, ls="--", color="gray")
    ax.axvspan(0, l1, color="gray", alpha=0.1)

    if i == 0:
        ax.legend(fontsize=12)
    ax.set_title(titles[i], fontsize=12)
    ax.set_xlim(0, 100)
    ax.set_ylim(5e-3)
    if i == 0:
        ax.set_ylabel(r"d$\sigma$/d$\Omega$ [mb/sr]")

fig.supxlabel(
    r"$\theta_{CM}$ [$\circ$]", x=0.55, fontsize=plt.rcParams["axes.labelsize"]
)
fig.savefig("./Outputs/xs_reco.png", dpi=300)
fig.savefig("./Outputs/xs_reco.pdf", dpi=300)

plt.show()
