import pyphysics as phys
import numpy as np
import re
import matplotlib.pyplot as plt
import matplotlib.axes as mplaxes


def read_agr(filename):
    datasets = []
    with open(filename, "r") as f:
        lines = f.readlines()

    cur_x, cur_y = [], []
    in_data = False

    for raw in lines:
        line = raw.strip()

        # start a new dataset when we encounter @target
        if line.startswith("@target"):
            # save existing dataset
            if cur_x:
                datasets.append((np.array(cur_x), np.array(cur_y)))
                cur_x, cur_y = [], []
            in_data = False
            continue

        # detect start of XY numeric block
        if re.match(r"@type\s+xy", line):
            in_data = True
            continue

        # collect numeric data while in data block
        if in_data:
            parts = line.split()
            if len(parts) >= 2:
                try:
                    xval = float(parts[0])
                    yval = float(parts[1])
                    cur_x.append(xval)
                    cur_y.append(yval)
                except ValueError:
                    # skip non-numeric lines
                    pass

    # append last dataset
    if cur_x:
        datasets.append((np.array(cur_x), np.array(cur_y)))

    return datasets


# Read data
lay = read_agr("./second.agr")
# Seems like his calculations are the last 4 elements
lay = lay[5:]

# Read OUR recalculations
our_labels = ["Our 2+1 0.39", "Our 2+2 0.18"]
our_files = ["./g1_Haixia/fort.202", "./g2_Haixia/fort.202"]
ours = [phys.parse_txt(file) for file in our_files]
his_labels = ["Lay 2+1 0.39", "Lay 2+2  0.18"]
his = [np.column_stack(lay[idx]) for idx in [0, -1]]  # found from grace file

# Get experimental data and fit
file_exps = ["../../Outputs/xs/g1_xs.dat", "../../Outputs/xs/g2_xs.dat"]
comps = [phys.Comparator(phys.parse_txt(file, 3)) for file in file_exps]
for i, comp in enumerate(comps):
    # Lay
    comp.add_model(his_labels[i], "", his[i])
    # Ours
    comp.add_model(our_labels[i], "", ours[i])
    # Fit
    comp.fit()

fig, axs = plt.subplots()

ax: mplaxes.Axes = axs
# Set log scale
ax.set_yscale("log")
# Lay
for vals in lay:
    ax.plot(vals[0], vals[1])
# Ours
for i, our in enumerate(ours):
    ax.plot(our[:, 0], our[:, 1], ls="--", label=our_labels[i])

ax.legend()
ax.set_xlabel(r"$\theta_{CM}$ [$\circ$]")
ax.set_ylabel(r"$d\sigma/d\Omega$ [mb/sr]")
fig.tight_layout()
fig.savefig("./Pictures/overlay.pdf", dpi=300)


fig, axs = plt.subplots(1, len(comps), figsize=(9, 4))
titles = ["2+1", "2+2"]
for i, comp in enumerate(comps):
    ax: mplaxes.Axes = axs[i]
    comp.draw(ax=ax, title=titles[i])
fig.tight_layout()
fig.savefig("./Pictures/scalings.pdf", dpi=300)


plt.show()
