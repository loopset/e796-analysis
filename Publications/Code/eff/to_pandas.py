import pandas as pd

file = "../../../Macros/PID/twosils.dat"

df = pd.read_csv(file, delimiter=" ", names=["run", "entry"])
# Create empty columns for efficiency
df["type"] = ""
df["status"] = False
print(df)
# Save
df.to_csv("twosils.csv", index=False)