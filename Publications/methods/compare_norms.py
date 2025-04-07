from shutil import which
import matplotlib.patches as mplpatches
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
import numpy as np
import uncertainties as un
import uncertainties.unumpy as unp
import pyphysics

# What we have measured
labels = [
    r"(d,d)$_{\mathrm{g.s}}$ scale",
    r"$M_{n}/M_{p}$ (d,d) $2^{+}_{1}$",
    r"(d,t)$_{\mathrm{g.s}}$ SF",
]

# Using pp uncertainty: fill_between
# Updated at 07/04/2025
pplow = [
    un.ufloat_fromstr("0.6862(99)"),
    un.ufloat_fromstr("1.420(63)"),
    un.ufloat_fromstr("2.714(60)"),
]
ppcenter = [
    un.ufloat_fromstr("0.894(12)"),
    un.ufloat_fromstr("1.784(71)"),
    un.ufloat_fromstr("3.535(78)"),
]
ppup = [
    un.ufloat_fromstr("1.269(16)"),
    un.ufloat_fromstr("2.363(88)"),
    un.ufloat_fromstr("5.02(11)"),
]
# OVERESTIMATION due to having considered custom weighting in xs
# pplow = [
#     un.ufloat_fromstr("0.686(89)"),
#     un.ufloat_fromstr("1.419(63)"),
#     un.ufloat_fromstr("2.712(60)"),
# ]
# ppcenter = [
#     un.ufloat_fromstr("0.911(12)"),
#     un.ufloat_fromstr("1.811(72)"),
#     un.ufloat_fromstr("3.602(77)"),
# ]
# ppup = [
#     un.ufloat_fromstr("1.326(17)"),
#     un.ufloat_fromstr("2.446(90)"),
#     un.ufloat_fromstr("5.24(12)"),
# ]


values = {
    "No condition": [
        un.ufloat_fromstr("0.4922(64)"),
        un.ufloat_fromstr("1.038(52)"),
        un.ufloat_fromstr("1.946(43)"),
    ],
    "(p,p) BG SF = 1": [
        un.ufloat_fromstr("0.6981(90)"),
        un.ufloat_fromstr("1.442(63)"),
        un.ufloat_fromstr("2.761(61)"),
    ],
    "(d,d) Daeh SF = 1": [
        un.ufloat_fromstr("1(0)"),
        un.ufloat_fromstr("1.956(77)"),
        un.ufloat_fromstr("3.954(87)"),
    ],
    r"$M_{n}/M_{p}$ (p,p) = E. Khan": ppcenter,
    r"$M_{n}/M_{p}$ (d,d) = E. Khan": [
        un.ufloat_fromstr("2.090(27)"),
        un.ufloat_fromstr("3.44(12)"),
        un.ufloat_fromstr("8.26(18)"),
    ],
}

fig, ax = plt.subplots(1, 1, figsize=(8, 4))  # type: ignore
ax: mplaxes.Axes
entries1 = []
for key, vals in values.items():
    eb = ax.errorbar(
        labels,
        unp.nominal_values(vals),
        yerr=unp.std_devs(vals),
        marker="s",
        ls="dashed",
        label=key,
    )
    entries1.append(eb)
# Filled region
entries1.append(
    ax.fill_between(
        labels,
        unp.nominal_values(pplow),
        unp.nominal_values(ppup),  # type: ignore
        color=entries1[3].lines[0].get_color(),
        alpha=0.15,
        label=r"d-breakup systematic",
    )
)

# Accepted regions
# Khan Mn/Mp
khan = un.ufloat(3.43, 0.42)
# A. Ramus SF
ramus = un.ufloat(4.70, 0.94) / 1.3  # type: ignore
names = [rf"E. Khan $M_{{n}}/M_{{p}} = $ {khan:%.2uS}", f"A. Ramus SF = {ramus:%.2uS}"]
colors = iter(plt.cm.Accent.colors[:2])  # type: ignore
entries2 = []
for i, (x, val) in enumerate(zip([1, 2], [khan, ramus])):
    rect = mplpatches.Rectangle(
        xy=(x - 0.25, val.n - val.s),
        width=0.5,
        height=2 * val.s,
        color=next(colors),
        alpha=0.6,
        label=names[i],
    )
    ax.add_patch(rect)
    entries2.append(rect)

ax.set_xlim(-0.5, len(labels) - 0.5)
ax.set_xlabel("Physics")
ax.set_ylabel("Value")
ax.tick_params(axis="x", which="minor", bottom=None, top=None)
# Legends
leg1 = ax.legend(
    handles=entries1,
    loc="upper left",
    fontsize=12,
    title="Norm. condition",
    title_fontsize=14,
)
leg2 = ax.legend(
    handles=entries2,
    loc="upper center",
    title="Previous measurements",
    fontsize=12,
    title_fontsize=14,
)
ax.add_artist(leg1)
fig.tight_layout()
fig.savefig("./Outputs/compare_norms_updated.png", dpi=200)
plt.show()
