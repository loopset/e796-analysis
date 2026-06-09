import pyphysics as phys
import matplotlib.pyplot as plt
import numpy as np
import hist
import sys

sys.path.append("../")
import styling as sty

plt.rcParams["axes.labelsize"] = 16

exs = [0, 2, 5, 10]
sigma = 0.350

h = hist.Hist.new.Reg(100, -2, 15, label=r"$E_{x}$ [MeV]").Double()

# Fill histogram with Gaussian samples for each excitation energy
# Number of events decreases with increasing ex
n_events_max = 1000
for i, ex in enumerate(exs):
    n_events = int(n_events_max * (1 - i * 0.2))
    samples = np.random.normal(ex, sigma, n_events)
    h.fill(samples)

fig, ax = plt.subplots(1, 1, figsize=(5, 4), constrained_layout=True)
h.plot(ax=ax, **sty.base1d, color="dodgerblue")
ax.set_ylabel("Counts")
fig.savefig("./Outputs/ex_explanation.png", dpi=300)

plt.show()
