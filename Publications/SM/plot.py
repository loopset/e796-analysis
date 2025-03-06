import sys

sys.path.append("../../Python/")
import matplotlib.pyplot as plt
import matplotlib as mpl

plt.style.use("../../Python/actroot.mplstyle")

from shell_model import SMPlotter

plotter = SMPlotter("b", False)
plotter.set_fillings("p", {"0p1/2": 2})
plotter.set_fillings("n", {"0d5/2": 4})

ax = plotter.plot(None, True)

# Highlight N  =8
r = mpl.patches.Rectangle((0.1, 0.9), 0.96, 0.25, lw=2, ec="orchid", fc="none")
ax.add_patch(r)
ax.set_xlim(xmax=1.1)


plt.gcf().savefig("./sm_l1.pdf")

plt.show()
