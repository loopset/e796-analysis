import shutil
import os
import re
import subprocess

# Determine state
state = "g2"

if state == "g2":
    par = "p2"
elif state == "g3":
    par = "p3"
else:
    raise ValueError("State can only be g2 or g3")

print("For state: ", state)

# Input file
initial = f"../../Inputs/{state}_Daeh/fresco.in"

# Values of coulomb deformation
if state == "g3":
    values = [0, 10, 20, 30, 40, 50]
else:
    values = [0, 2, 5, 7, 9, 11]

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
