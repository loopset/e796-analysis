import pyphysics as phys
import matplotlib.pyplot as plt
from adjustText import adjust_text

sfo = phys.ShellModel(
    [
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j0n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j0p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j2n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j2p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j4n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j4p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j6n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j6p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j8n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j8p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j10n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j10p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j12n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j12p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j14n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j14p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j16n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j16p_j3p.txt",
    ],
    is_adding=True,
)

# Set min C2S
sfo.set_min_SF(0.05)

# Define orbital of transferred nucleon
qd32 = phys.Orbital.from_str("0d3/2")
qd52 = phys.Orbital.from_str("0d5/2")
qs12 = phys.Orbital.from_str("1s1/2")

# Get strength and max Ex
for q, lis in sfo.data.items():
    if not lis:
        continue
    # Sum WITHOUT CONSIDERING SPIN FACTOR
    stre = sum(val.SF for val in lis)  # type: ignore
    max_ex = max(val.Ex for val in lis)  # type: ignore
    print(
        f"Strength for {q.format_simple()} = {stre:.3f} from Ex [0, {max_ex:.3f}] MeV"
    )
    # Sum CONSIDERING SPIN FACTOR
    method = sfo.sum_strength(q)
    print("  with spin factor = ", method)
    print(f"  from ShellModel class: {sfo.sum_strength(q)}")

# Group by Ex and Jpif
groupexjpi = sfo.group_by_Ex_and_Jpif()
twoplus = sfo.group_by_Jpif(groupexjpi)["2+"]
# print(groupexjpi)
# print(twoplus)


fig, ax = plt.subplots(constrained_layout=True, figsize=(4.5, 3))
_, texts = sfo.plot_bars(groupexjpi, ax=ax, fontsize=10, right_padding=0.1, left_padding=0.1)
ax.set_xticks([0], ["SFO-tls"])
ax.legend(ncol=1, loc = "upper left", fontsize=12)
ax.set_xlim(-0.9, 0.65)

ax.spines["right"].set_visible(False)
ax.spines["top"].set_visible(False)
ax.spines["bottom"].set_visible(False)
ax.tick_params(axis="x", which="both", bottom=False, top=False, labelbottom=True)
ax.tick_params(axis="y", which="both", right=False)

# Sn and S2n
c18 = phys.Particle("18C")
ax.axhline(c18.get_sn(), ls="--", color="gray")
ax.axhline(c18.get_s2n(), ls="--", color="pink")
ax.annotate(r"$S_n$", xy=(-0.8, c18.get_sn() - 0.5), ha="center", va="center", fontsize=12)
ax.annotate(r"$S_{2n}$", xy=(-0.8, c18.get_s2n() + 0.5), ha="center", va="center", fontsize=12)

# Title
ax.set_title(r"$^{18}$C", fontsize=14, fontweight="bold")


positions = [t.get_position() for t in texts]
# adjust_text(
#     texts,
#     target_x=[p[0] for p in positions],
#     target_y=[p[1] for p in positions],
#     avoid_self=False,
#     only_move=dict(text="y", static="y", explode="y", pull="y"),
#     # arrowprops=dict(arrowstyle="-", color="crimson", ls="dotted"),
# )

# Save
fig.savefig("./Outputs/sfo-tls.pdf", dpi=300)
fig.savefig("./Outputs/sfo-tls.png", dpi=300)

plt.show()
