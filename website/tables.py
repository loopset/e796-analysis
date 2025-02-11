import ROOT as r
import json
import uncertainties as unc

# List the directories
path = "/media/Data/E796v2/Fits/"
files = {"20O(p,p)": "pp", "20O(d,d)": "dd", "20O(d,t)": "dt", "20O(p,d)": "pd"}


# Function to extract data
def extract(dir: str) -> dict:
    # Use interface to get Ex
    inter = r.Fitters.Interface()
    inter.Read(
        path + dir + "/Outputs/interface.root",
        path + dir + "/Outputs/fit_juan_RPx.root",
    )
    # Open the file
    file = r.TFile(path + dir + "/Outputs/sfs.root")
    states = file.Get("Keys")
    sfs = {}
    for state in states:
        col = file.Get(f"{state}_sfs")
        data = []
        ex = unc.ufloat(inter.GetParameter(state, 1), inter.GetUnc(state, 1))
        for _, name in enumerate(col.GetModels()):
            sf_obj = col.Get(name)
            ## Define the values to be printed to table
            sf = unc.ufloat(sf_obj.GetSF(), sf_obj.GetUSF())
            data.append(
                [
                    "{:.2uS}".format(ex),
                    name.decode("utf-8"),
                    "{:.2uS}".format(sf),
                    round(sf_obj.GetChi2Red(), 2),
                ]
            )
        sfs[state.decode("utf-8")] = data
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
                    row["Ex"] = val
                elif i == 1:
                    row["Model"] = val
                elif i == 2:
                    row["SF"] = val
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
    merged[key] = [d, other[0]]
with open("tables.json", "w") as json_file:
    json.dump(merged, json_file, indent=4)
