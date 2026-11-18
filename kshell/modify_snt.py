from typing import Tuple, Dict
from pathlib import Path

TBMEKey = Tuple[int, int, int, int, int]

shifts_spe: Dict[int, float] = {
    # Proton
    1: +2.0,
    2: +2.0,
    3: +2.0,
    4: +2.0,
    5: +2.0,
    # Neutron
    6: +2.0,
    7: +2.0,
    8: +2.0,
    9: +2.0,
    10:+2.0,
}

shifts_tbme: Dict[TBMEKey, float] = {
    # 0p1/2
    (1, 1, 1, 1, 0): -0.5,
    (6, 6, 6, 6, 0): -0.5,
    (1, 6, 1, 6, 0): -0.5,
    # 0d5/2
    (4, 4, 6, 4, 0): -0.5,
    (9, 9, 9, 9, 0): -0.5,
    (4, 9, 4, 9, 0): -0.5,
}


def apply(
    infile: str,
    outfile: str,
    shifts_spe: Dict[int, float],
    shifts_tbme: Dict[TBMEKey, float],
) -> None:
    input = Path(infile)
    out = []
    keys_spe = set(shifts_spe)
    keys_tbme = set(shifts_tbme)
    for line in input.read_text().splitlines():
        cols = line.split()
        ################################################
        # SPE
        if len(cols) == 3 and all(c.isdigit() for c in cols[:2]) and cols[0] == cols[1]:
            key: int = int(cols[0])  # type: ignore
            if key in shifts_spe:
                print("======== SPE ==========")
                print(line)
                value = float(cols[-1])
                new_value = value + shifts_spe[key]
                print(f"  value : {value}")
                print(f"  new   : {new_value}")
                line = "{:3d}{:4d}{:16.5f}".format(key, key, new_value)
                keys_spe.discard(key)
        ################################################
        # TBME
        if len(cols) == 6 and all(c.isdigit() for c in cols[:5]):
            key: TBMEKey = tuple(map(int, cols[:5]))  # type: ignore
            if key in shifts_tbme:
                print("====== TBME =======")
                print(line)
                value = float(cols[-1])
                new_value = value + shifts_tbme[key]
                print(f"  value : {value}")
                print(f"  new   : {new_value}")
                line = "{:3d}{:4d}{:4d}{:4d}{:5d}{:18.5f}".format(*key, new_value)
                keys_tbme.discard(key)
        out.append(line)
    # Write
    output = Path(outfile)
    output.write_text("\n".join(out) + "\n")

    return


stn = "sfotls_modtnew"
input = f"/home/miguel/kshell/snt/{stn}.snt"
output = f"./20O_tbme_spe/4hw/{stn}.snt"
apply(input, output, shifts_spe, shifts_tbme)
print(f"Modified {input} -> {output}")
