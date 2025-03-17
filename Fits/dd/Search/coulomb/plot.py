import ROOT as r
import os
import re

# Determine state
state = "g3"

if state == "g2":
    par = "p2"
elif state == "g3":
    par = "p3"
else:
    raise ValueError("State can only be g2 or g3")
# Read experimental xs
with r.TFile("../../Outputs/xs.root") as f:
    gexp = f.Get(f"g{state}")

# And plot
comp = r.Angular.Comparator("Coulomb comparison", gexp)

pattern = re.compile(rf"{state}_coulomb_(\d+(\.\d+)?)")

for subdir in os.listdir("./"):
    path = os.path.join("./", subdir)
    if os.path.isdir(path):
        fresco = os.path.join(path, "fort.202")
        if os.path.exists(fresco):
            match = pattern.match(subdir)
            if match:
                comp.Add("#sqrt{B(EL)} = " + match.group(1), fresco)
comp.Fit()
comp.Draw()
