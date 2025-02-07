import ROOT as r
import json

# List the directories
path = "/media/Data/E796v2/Fits/"
files = {"20O(p,p)": "pp", "20O(d,d)": "dd", "20O(d,t)": "dt", "20O(p,d)": "pd"}

# Function to extract data
def extract(dir: str) -> dict:
    # Open the file
    file = r.TFile(path + dir + "/Outputs/sfs.root")
    states = file.Get("Keys")
    sfs = {}
    for state in states:
        col = file.Get(f"{state}_sfs")
        data = []
        for _, name in enumerate(col.GetModels()):
            sf = col.Get(name)
            data.append([name.decode('utf-8'), round(sf.GetSF(), 3), round(sf.GetUSF(), 3), round(sf.GetChi2Red(), 2)])
        sfs[state.decode('utf-8')] = data
    return sfs


all = {}
for header, dir in files.items():
    d = extract(dir)
    table = []
    for state, vals in d.items():
        for model in vals:
            row = {"State": state}
            for i, val in enumerate(model):
                if i == 0:
                    row["Model"] = val
                elif i == 1:
                    row["SF"] = val
                elif i == 2:
                    row["uSF"] = val
                elif i == 3:
                    row["Chi2red"] = val
            table.append(row)
    all[header] = {"title": "SFs (p norm)", "data": table}


# Append d norm data
with open("tables_dnorm.json", "r") as other:
    dnorm = json.load(other)

merged = {}
for key, d in all.items():
    other = {}
    if key in dnorm:
        other = dnorm[key]
    merged[key] = [d, other]
with open("tables.json", "w") as json_file:
    json.dump(merged, json_file, indent=4)
