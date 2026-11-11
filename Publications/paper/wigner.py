import pyphysics as phys
import numpy as np
import matplotlib.pyplot as plt


# Wigner term estimation for 16O
def delta_v(Z: int, N: int) -> float:
    pzn = phys.Particle.from_numbers(Z + N, Z)
    pzm2nm2 = phys.Particle.from_numbers(Z - 2 + N - 2, Z - 2)
    pznm2 = phys.Particle.from_numbers(Z + N - 2, Z)
    pzm2n = phys.Particle.from_numbers(Z - 2 + N, Z - 2)
    return pzn.get_be() - pzm2n.get_be() - pznm2.get_be() + pzm2nm2.get_be()


def delta_k(Z: int, N: int) -> float:
    pzn = phys.Particle.from_numbers(Z + N, Z)
    pzm2nm1 = phys.Particle.from_numbers(Z - 2 + N, Z - 2)
    pznm2 = phys.Particle.from_numbers(Z + N - 1, Z)
    pzm2nm2 = phys.Particle.from_numbers(Z - 2 + N - 1, Z - 2)
    return pzn.get_be() - pzm2nm1.get_be() - pznm2.get_be() + pzm2nm2.get_be()


def e_smooth_even(Z: int, N: int) -> float:
    return 0.25 * (
        delta_v(Z - 2, N) + delta_v(Z + 2, N) + delta_v(Z, N - 2) + delta_v(Z, N + 2)
    )


def e_smooth_odd(Z: int, N: int) -> float:
    return 0.25 * (
        delta_k(Z - 2, N) + delta_k(Z + 2, N) + delta_k(Z, N - 2) + delta_k(Z, N + 2)
    )


def delta_n(Z: int, N: int) -> float:
    delta_v_tilde = delta_v(Z, N) - e_smooth_even(Z, N)
    delta_k_tilde = delta_k(Z, N - 1) - e_smooth_odd(Z, N - 1)
    return 3 / 2 * (delta_v_tilde - delta_k_tilde)


# 40Ca
Z = 20
N = 20

print("========== 40Ca example ===========")
print(f"DeltaV(20,20) : {delta_v(Z, N)}")
print(f"DeltaK(20,19): {delta_k(20, 19)}")
print("E_smooth(20, 20) : ", e_smooth_even(Z, N))
print(f"E_smooth(20,19): {e_smooth_odd(Z, 19)}")
print(f"DeltaN(20,20): {delta_n(Z, N)}")
print("===================================")

# 16O
Z = 8
N = 8
print("========== 16O ===========")
print(f"DeltaV(8,8) : {delta_v(Z, N)}")
print(f"DeltaK(8,7): {delta_k(Z, N - 1)}")
print("E_smooth(8, 8) : ", e_smooth_even(Z, N))
print(f"E_smooth(8,7): {e_smooth_odd(Z, N - 1)}")
print(f"DeltaN(8,8): {delta_n(Z, N)}")
print("===================================")
