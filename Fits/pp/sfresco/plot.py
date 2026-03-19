import pyphysics as phys
import matplotlib.pyplot as plt
import numpy as np
import pathlib
import uncertainties as un

# Experimental angular
# exp = phys.parse_txt("../Outputs/xs/g0_xs.dat", 3)
exp = np.array(
    [
        [18.5, 1122.9 / 0.791, 14.5255 / 0.791],
        [19.5, 1030.98 / 0.791, 13.5448 / 0.791],
        [20.5, 779.318 / 0.791, 11.2043 / 0.791],
        [21.5, 609.292 / 0.791, 9.50173 / 0.791],
        [22.5, 508.885 / 0.791, 8.48533 / 0.791],
        [23.5, 408.907 / 0.791, 7.40804 / 0.791],
    ]
)

# Directory containing all cte_*_* subdirs
parent_dir = pathlib.Path("./Outputs/")

# Collect all subdirs matching pattern
subdirs = sorted(
    [d for d in parent_dir.iterdir() if d.is_dir() and d.name.startswith("cte_")]
)

data = []

for d in subdirs:
    # Extract cte0, cte1 from dir name: cte_0p8_0p5 → 0.8, 0.5
    _, c0_str, c1_str = d.name.split("_")
    cte0 = float(c0_str.replace("p", "."))
    cte1 = float(c1_str.replace("p", "."))

    # --- Custom calculation ---
    # Example: read a file z.dat and sum numbers
    xs = d / "fort.201"  # replace with your actual file
    comp = phys.Comparator(exp)
    comp.add_model("this", str(xs))
    comp.fit()
    sf = comp.get_sf("this").nominal_value
    chi = comp.fChis["this"]

    data.append([cte0, cte1, sf])

grid = np.array(data)

# Sort first by cte1 (rows), then by cte0 (columns)
# This ensures consistent 2D ordering
grid_sorted = grid[np.lexsort((grid[:,0], grid[:,1]))]

# Unique x and y values
x_vals = np.unique(grid_sorted[:,0])
y_vals = np.unique(grid_sorted[:,1])

# Reshape z into 2D array
Z = grid_sorted[:,2].reshape(len(y_vals), len(x_vals))  # rows=y, cols=x

# Tolerance for z≈1
tol = 0.01

# Find positions where z is close to 1
ys, xs = np.where(np.abs(Z - 1.0) <= tol)  # indices in 2D array
x_pts = x_vals[xs]
y_pts = y_vals[ys]

# Plot
fig, ax = plt.subplots()
pcm = ax.pcolormesh(x_vals, y_vals, Z, shading='auto', cmap='viridis')
ax.scatter(x_pts, y_pts, color='red', marker='*', s=100, label='z≈1')
ax.set_xlabel("cte0")
ax.set_ylabel("cte1")
fig.colorbar(pcm, ax=ax, label="z value")
ax.legend()
plt.show()
