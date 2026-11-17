import pyphysics as phys
import matplotlib.pyplot as plt
import numpy as np

Ex = np.array(
    [
        4.3,
        4.5,
        4.7,
        4.9,
        5.1,
        5.3,
        5.5,
        5.7,
        5.9,
        6.2,
        6.5,
        6.8,
        7.1,
        7.6,
        8.1,
        8.6,
        9.1,
        9.6,
        10.1,
        10.6,
        11.1,
        11.6,
        12.1,
        12.6,
        13.1,
        13.6,
        14.1,
        14.6,
        15.1,
        15.6,
    ]
)

# Er = Ex - Sn
Er = np.array(
    [
        0.12,
        0.32,
        0.52,
        0.72,
        0.92,
        1.12,
        1.32,
        1.52,
        1.72,
        2.02,
        2.32,
        2.62,
        2.92,
        3.42,
        3.92,
        4.42,
        4.92,
        5.42,
        5.92,
        6.42,
        6.92,
        7.42,
        7.92,
        8.42,
        8.92,
        9.42,
        9.92,
        10.42,
        10.92,
        11.42,
    ]
)

Gammasp = np.array(
    [
        0.842e-3,
        0.917e-2,
        0.292e-1,
        0.627e-1,
        0.111,
        0.175,
        0.255,
        0.353,
        0.469,
        0.679,
        0.932,
        1.24,
        1.61,
        2.26,
        3.13,
        4.29,
        5.60,
        7.12,
        9.27,
        11.9,
        14.4,
        17.5,
        22.5,
        29.2,
        35,
        40.4,
        50.5,
        71.0,
        99.9,
        119,
    ]
)

fig, ax = plt.subplots(constrained_layout=True, figsize=(5,3.5))
ax.plot(Ex, 0.25 * Gammasp, marker="s", ms=5, ls="-", color="orange")
ax.set_xlabel(r"$E_x$ [MeV]")
ax.set_ylabel(r"$0.25 \cdot \Gamma_{sp}$ [MeV]")
ax.set_title(r"$\Gamma_{sp}$ for $^{17}C(d,p)^{18}$C")
fig.savefig("./Outputs/widths.pdf", dpi=300)

plt.show()

