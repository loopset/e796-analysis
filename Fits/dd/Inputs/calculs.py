import math
import sys

# 20O
a = 20
z = 8
n = a - z


def main(be: float, l: int) -> None:
    coulomb = math.sqrt(be)
    r = 1.2 * a ** (1 / 3)
    nuclear = 4 * math.pi / (3 * z * r**l)
    beta = nuclear * coulomb

    print(f"B(E{l})   : ", be)
    print(f"R0      : {r:.2e}")
    print(f"Coulomb : {coulomb:.2e}")
    print(f"Beta    : {beta:.2e}")
    return


if __name__ == "__main__":
    args = sys.argv[1:]
    be = float(args[0])
    l = int(args[1])
    main(be, l)
