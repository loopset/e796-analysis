from typing import Tuple, Dict
from pathlib import Path

Key = Tuple[int, int, int, int, int]

shifts: Dict[Key, float] = {
    # 0p1/2
    (1, 1, 1, 1, 0): 1.2,
    (6, 6, 6, 6, 0): 1.2,
    (1, 6, 1, 6, 0): 1.2,
    # 0d5/2
    (4, 4, 4, 4, 0): -5.0,
    (9, 9, 9, 9, 0): -5.0,
    (4, 9, 4, 9, 0): -5.0,
}


def apply(infile: str, outfile: str, shifts: Dict[Key, float]) -> None:
    input = Path(infile)
    out = []
    keys = set(shifts)
    for line in input.read_text().splitlines():
        cols = line.split()
        if len(cols) == 6 and all(c.isdigit() for c in cols[:5]):
            key: Key = tuple(map(int, cols[:5]))  # type: ignore
            if key in shifts:
                print("==================")
                print(line)
                value = float(cols[-1])
                new_value = value + shifts[key]
                print(f"  value : {value}")
                print(f"  new   : {new_value}")
                line = "{:3d}{:4d}{:4d}{:4d}{:5d}{:18.5f}".format(*key, new_value)
                keys.discard(key)
        out.append(line)
    # Write
    output = Path(outfile)
    output.write_text("\n".join(out) + "\n")

    return


stn = "sfotls_modtnew"
print(f"Modifying {stn} file")
input = f"/home/miguel/kshell/snt/{stn}.snt"
output = f"./it6/2hw/{stn}.snt"
apply(input, output, shifts)
