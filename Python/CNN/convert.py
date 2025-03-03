import ROOT as r
import numpy as np
import h5py

# Read one TTree
file = r.TFile("/media/Other/E796/RootFiles/Data/Clusters_Run_0155.root")
tree = file.Get("ACTAR_Clusters")
d = r.ActRoot.TPCData()
tree.SetBranchAddress("TPCData", d)

x = []
y = []
z = []
entries = tree.GetEntries()
for entry in range(entries):
    tree.GetEntry(entry)
    entry_x = []
    entry_y = []
    entry_z = []
    for cl in d.fClusters:
        for v in cl.GetVoxels():
            pos = v.GetPosition()
            entry_x.append(int(pos.X()))
            entry_y.append(int(pos.Y()))
            entry_z.append(int(pos.Z()))
    x.append(entry_x)
    y.append(entry_y)
    z.append(entry_z)

# Save as hf5
ax = np.array(x, dtype=object)
ay = np.array(y, dtype=object)
az = np.array(z, dtype=object)

# Save to file
with h5py.File('run_155.h5', 'w') as f:
    f.create_dataset('x', data=ax)
    f.create_dataset('y', data=ay)
    f.create_dataset('z', data=az)