import pyphysics as phys
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as stats

# Intensity
i = 2.5e4
# Time window
w = 41e-6

# Mean
mu = i * w

poisson = stats.poisson(mu=mu)
mean = poisson.mean()
print(mean)

fig, ax = plt.subplots(1, 1, figsize=(4, 3), constrained_layout=True)
x = np.arange(start=0, stop=5, step=1)
ax.bar(x, poisson.pmf(x), width=0.25, color="dodgerblue")
ax.axvline(mean, ls="--", color="crimson", label=rf"$\mu$ = {mean:.1f}")
ax.set_xlabel("# pile-up")
ax.set_ylabel("pdf")
ax.set_ylim(0, 0.5)
ax.legend()

fig.savefig("./Outputs/poisson.png", dpi=300)

plt.show()
