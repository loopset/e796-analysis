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
    r"$M_{n}/M_{p} =$ E. Khan": [
        un.ufloat_fromstr("2.090(27)"),
        un.ufloat_fromstr("3.44(12)"),
        un.ufloat_fromstr("8.26(18)"),
    ],
}

fig, ax = plt.subplots(1, 1, figsize=(7, 4))  # type: ignore
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

# Accepted regions
# Khan Mn/Mp
khan = un.ufloat(3.43, 0.42)
# A. Ramus SF
ramus = un.ufloat(4.70, 0.94) / 1.3  # type: ignore
names = [fr"E. Khan $M_{{n}}/M_{{p}} = $ {khan:%.2uS}", f"A. Ramus SF = {ramus:%.2uS}"]
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
fig.savefig("./Outputs/compare_norms.png", dpi=200)
plt.show()
