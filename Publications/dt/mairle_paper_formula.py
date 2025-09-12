import pyphysics as phys
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes


def eso_splitting(z: int, a: int, t: float, t0: float, isProton: bool = False) -> float:
    eso16O = 6.5  # from G. Mairle paper of 1974 in page 254
    da = -0.6  # MeV
    db = -3.38
    dc = -0.10 if isProton else 0
    eso = (
        eso16O
        - (a - 16) * da
        + 0.5 * db * (t * (t + 1) - t0 * (t0 + 1) - 3.0 / 4)
        + (z - 8) * dc
    )
    return eso


# In our case
# Target: 20O, T0 = 2
target = phys.Particle("20O")
t0 = 2

# Do for 19O and 19N
labels = ["T = 3/2", "T = 5/2"]
ts = [3.0 / 2, 5.0 / 2]
esos = []
for i, label in enumerate(labels):
    eso = eso_splitting(target.Z, target.A, ts[i], t0)
    esos.append(eso)

# Plot
fig, ax = plt.subplots()
ax.plot(labels, esos, marker="s", color="green")
ax.set_ylabel(r"$\epsilon_{\text{so}}$ G. Mairle (1974) formula [MeV]")

plt.show()
