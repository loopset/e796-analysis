import shutil
import os
import re
import subprocess

# State name
state = "g5"

# Parameter to replace
par = "p3"

# Input file
initial = "./fresco.in"

# Values of Coulomb deformation
values = [0, 1, 5, 10, 15, 20, 40, 60, 80, 100]

# Store subdirs info
subdirs = []

for value in values:
    subdir = os.path.join("./", f"{state}_coulomb_{value}")
    subdirs.append(subdir)
    os.makedirs(subdir, exist_ok=True)

    # Copy fresco to that path
    fresco = os.path.join(subdir, "fresco.in")
    shutil.copy(initial, fresco)

    # Modify it
    with open(fresco, "r") as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if re.match(r"^\s*&POT\s+kp=1\s+type=11", line):
            lines[i] = f" &POT kp=1 type=11 {par}={value} / ! Modified\n"
            break
    with open(fresco, "w") as f:
        f.writelines(lines)

    print(f" sqrt(B(EL)) : {value}")

    # Execute fresco
    subprocess.run(
        ["fresco <fresco.in> fresco.out"],
        cwd=subdir,
        shell=True,
        capture_output=True,
        text=True,
    )
