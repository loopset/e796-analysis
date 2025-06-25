import pyphysics as phys
import pickle
import matplotlib.pyplot as plt
import uncertainties as un

# Quantum numbers
qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)
# Removal
snrem = phys.Particle("18O").get_sn()
# Read
files = ["./Outputs/mairle.pkl", "./Outputs/reanalysis.pkl"]
gaps = []
for file in files:
    data: phys.SMDataDict = {}
    with open(file, "rb") as f:
        data = pickle.load(f)
    # Barager calculus
    bar = phys.Barager()
    bar.set_removal(data, snrem)
    bar.do_for([qp12, qp32])
    gap = bar.get_gap(qp12, qp32)
    gaps.append(gap)

fig, ax = plt.subplots()
labels = ["Mairle", "Reanalysis"]
for i, (label, gap) in enumerate(zip(labels, gaps)):
    print(f"Gap for {label} is {gap}")
    if i == 0:
        ax.hlines(un.nominal_value(gap), xmin=-0.25, xmax=0.25, color="crimson", label=label, lw=2)
    else:
        ax.hlines(un.nominal_value(gap), xmin=-0.25, xmax=0.25, color="dodgerblue", lw=2, label=label)
        ax.axhspan(
            un.nominal_value(gap) - un.std_dev(gap),
            un.nominal_value(gap) + un.std_dev(gap),
            xmin=0.25,
            xmax=0.75,
            color="dodgerblue",
            alpha=0.5,
        )
ax.legend()
ax.set_xticks([0], ["SO gap"])
ax.set_xlim(-0.5, 0.5)
fig.tight_layout()
plt.show()
