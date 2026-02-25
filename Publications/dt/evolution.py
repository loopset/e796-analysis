from collections import defaultdict
from re import S
from typing import List, Union

from matplotlib.lines import Line2D
import pyphysics as phys
import uncertainties as un
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes
from matplotlib.patches import Patch, Rectangle
import copy
import pickle
from adjustText import adjust_text

import sys

sys.path.append("./")
sys.path.append("../")

import dt
import styling as sty

############################################### 20O(d,t)
# Experiment
exp = dt.build_sm()

exp_sfos = dt.build_theos(gated=True)[:-1]

############################################## 18O(d,t)
# Experiment
with open("../../Fits/dt/Mairle_18Odt/Outputs/reanalysis.pkl", "rb") as f:
    # with open("../../Fits/dt/Mairle_18Odt/Outputs/mairle.pkl", "rb") as f:
    dic = pickle.load(f)
    mairle = phys.ShellModel()
    mairle.data = dic

mairle_sfos = dt.build_mairle_theos(gated=True)

############################################## 16O(d,t)
# Experiment
with open("../../Fits/dt/16O_dt/exp.pkl", "rb") as f:
    exp16 = pickle.load(f)

# Plain SFO-tls
with open("../../Fits/dt/16O_dt/mod0.pkl", "rb") as f:
    o16_sfo = pickle.load(f)

# SFO-tls mod1
with open("../../Fits/dt/16O_dt/mod1.pkl", "rb") as f:
    o16_sfo1 = pickle.load(f)

# Labels
labels = [
    r"$^{16}$O Exp.",
    r"$^{16}$O SFO-tls",
    r"$^{16}$O Mod1",
    r"$^{18}$O Exp.",
    r"$^{18}$O SFO-tls",
    r"$^{18}$O Mod1",
    r"$^{20}$O Exp.",
    r"$^{20}$O SFO-tls",
    r"$^{20}$O Mod1",
]

# Models up to 20O
models = [exp16, o16_sfo, o16_sfo1, mairle]
for i, m in enumerate(models):
    if isinstance(m, phys.ShellModel):
        m.set_min_SF(dt.minC2S)  # exp threshold
        m.set_max_Ex(dt.maxEx)  # same cut as Exp 20O
# Add mairle theos
models = [*models, *mairle_sfos]
# Add 20O data
models = [*models, exp, *exp_sfos]
dicts = [getattr(m, "data", m) for m in models]


def compute(models: List[phys.SMDataDict]) -> tuple:
    stes = []
    cents = []
    gaps = []
    for model in models:
        st = dt.get_strengths(model)
        # Push strengths
        stes.append(st)
        cs = dt.get_centroids(model)
        # Push centroids
        cents.append(cs)
        # Push gaps
        gap = abs(cs[dt.qp12] - cs[dt.qp32])  # type: ignore
        gaps.append(gap)

    return (stes, cents, gaps)


stes, cents, gaps = compute(dicts)

with open("./Inputs/evolution.pkl", "wb") as f:
    pickle.dump((stes, cents, gaps), f)


# Higher C2S threshold for visual plot
clones = [copy.deepcopy(m) for m in models]
clones_dicts = []
for clone in clones:
    if hasattr(clone, "data"):
        clone.set_min_SF(0.07)
    clones_dicts.append(getattr(clone, "data", clone))
# Vertical plots
nuclei = [r"$^{16}$O", r"$^{18}$O", r"$^{20}$O"]
titles = ["Exp", "SFO-tls", "Mod1 SFO-tls"]
for i in range(3):
    fig, ax = plt.subplots(figsize=(8, 4.5))
    texts = dt.plot_bars(clones_dicts[i::3], nuclei, ax, ilabel=2, height=0.15)
    ax.legend()
    ax.set_title(titles[i])
    ax.set_ylabel(r"$E_x$ [MeV]")
    ax.tick_params(axis="x", labelsize=18)
    fig.tight_layout()
    positions = [t.get_position() for t in texts]
    adjust_text(
        texts,
        target_x=[p[0] for p in positions],
        target_y=[p[1] for p in positions],
        avoid_self=False,
        only_move=dict(text="y", static="y", explode="y", pull="y"),
        # arrowprops=dict(arrowstyle="-", color="crimson", ls="dotted"),
    )
    fig.savefig(sty.thesis + f"vertical_evolution_{i}.pdf", dpi=300)

# Declare groups of models belonging to the same nucleus: list of lists
idxs = [[0, 1, 2], [3, 4, 5], [6, 7, 8]]
ls = ["-", "--", "dashdot"]
markers = [None, "s", "o"]
fig, axs = plt.subplots(1, 2, figsize=(9, 3.5), constrained_layout=True)

# Centroids
ax: mplaxes.Axes = axs[0]
for counter, lis in enumerate(idxs):
    xs = [counter - 0.4, counter + 0.4]
    for q in [dt.qp12, dt.qp32]:
        color = sty.barplot[q]["ec"]
        for i, idx in enumerate(lis):
            if idx is None:
                continue
            y = un.nominal_value(cents[idx][q])
            uy = un.std_dev(cents[idx][q])
            if i == 0:
                ax.plot(xs, [y] * 2, color=color, ls=ls[i])
                ax.fill_between(
                    xs, y1=[y - uy] * 2, y2=[y + uy] * 2, color=color, alpha=0.25
                )
            else:
                div = (xs[-1] - xs[0]) / (len(lis))
                ax.plot(xs[0] + i * div, y, marker=markers[i], color=color)
ax.set_ylabel(r"$\langle E_{x}\rangle$ [MeV]")

# Gaps
ax = axs[-1]
color = "green"
lines = defaultdict(list)
for counter, lis in enumerate(idxs):
    xs = [counter - 0.4, counter + 0.4]
    for i, idx in enumerate(lis):
        if idx is None:
            continue
        y = un.nominal_value(gaps[idx])
        uy = un.std_dev(gaps[idx])
        if i == 0:
            ax.plot(xs, [y] * 2, color=color, ls=ls[i])
            ax.fill_between(
                xs, y1=[y - uy] * 2, y2=[y + uy] * 2, color=color, alpha=0.25
            )
        else:
            div = (xs[-1] - xs[0]) / (len(lis))
            x = xs[0] + i * div
            ax.plot(x, y, marker=markers[i], color=color)
            lines[i].append((x, y))
# Draw lines
ls = {1: "dashdot", 2: "--"}
for idx, line in lines.items():
    ax.plot(*zip(*line), ls=ls[idx], marker="none", color=color, lw=0.75, zorder=1)
ax.set_ylabel(r"$\Delta_{N = 6}$ [MeV]")

# Common axis settings
for i, ax in enumerate(axs):
    ax.tick_params(axis="x", which="both", bottom=False, top=False)
    ax.tick_params(axis="x", labelsize=18, pad=7)
    ax.set_xticks([0, 1, 2])
    ax.set_xticklabels([r"$^{16}$O", r"$^{18}O$", r"$^{20}$O"])

# Custom legends
leg = axs[0].legend(
    handles=[
        Rectangle(
            xy=(0, 0),
            height=0,
            width=0,
            color=sty.barplot[dt.qp12]["ec"],
            ec="none",
            alpha=0.75,
            label=dt.qp12.format(),
        ),
        Rectangle(
            xy=(0, 0),
            height=0,
            width=0,
            color=sty.barplot[dt.qp32]["ec"],
            ec="none",
            alpha=0.75,
            label=dt.qp32.format(),
        ),
    ],
    loc=(0.4, 0.01),
    handlelength=1.5,
    fontsize=14,
)
axs[0].legend(
    handles=[
        Rectangle(xy=(0, 0), width=0, height=0, color="grey", alpha=0.35, label="Exp."),
        Line2D([], [], marker=markers[1], ls="none", color="grey", label="SFO-tls"),
        Line2D([], [], marker=markers[2], ls="none", color="grey", label="Mod1"),
    ],
    handlelength=1.5,
    fontsize=14,
    loc=(0.65, 0.01),
)
axs[0].add_artist(leg)

fig.savefig(sty.thesis + "evolution.pdf", dpi=300)


plt.show()
