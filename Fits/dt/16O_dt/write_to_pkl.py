import pyphysics as phys

import pickle

# Quantum number
qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)

# Experimental dataset
exp = phys.ShellModel()
exp.data[qp12] = [phys.ShellModelData(0, 0)]
exp.data[qp32] = [phys.ShellModelData(6.196, 0)]

# SFO-tls
sfo = phys.ShellModel()
sfo.data[qp12] = [phys.ShellModelData(0, 1.890)]
sfo.data[qp32] = [phys.ShellModelData(6.856, 3.843)]

# SFO-tls mod1
sfo1 = phys.ShellModel()
sfo1.data[qp12] = [phys.ShellModelData(0, 1.843)]
sfo1.data[qp32] = [phys.ShellModelData(6.595, 3.709)]

# SFO-tls mod2
sfo2 = phys.ShellModel()
sfo2.data[qp12] = [phys.ShellModelData(0, 1.832)]
sfo2.data[qp32] = [phys.ShellModelData(5.115, 3.687)]

# Filenames
names = ["exp", "mod0", "mod1", "mod2"]
models = [exp, sfo, sfo1, sfo2]

for name, model in zip(names, models):
    with open(f"./{name}.pkl", "wb") as f:
        pickle.dump(model, f)
