import pyphysics as phys
import matplotlib.pyplot as plt
import sys

sys.path.append("../")
sys.path.append("./")
import styling as sty
import dt


path = "/media/Data/E796v2/Fits/dt/Inputs/"
# Modified1 SFO-tls
sfo1 = phys.ShellModel(
    [
        path + "SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1n.txt",
        path + "SM_fited/log_O20_O19_sfotls_mod_tr_j0p_m1p.txt",
    ]
)
# Check mod1
new = phys.ShellModel(
    [
        "/media/Data/E796v2/Fits/dt/Inputs/Check/log_O20_O19_sfotls_modt_tr_m0p_j1n.txt",
        "/media/Data/E796v2/Fits/dt/Inputs/Check/log_O20_O19_sfotls_modt_tr_m0p_j3n.txt",
        "/media/Data/E796v2/Fits/dt/Inputs/Check/log_O20_O19_sfotls_modt_tr_m0p_m1p.txt",
    ]
)

for theo in [sfo1, new]:
    theo.set_min_SF(0.07)

fig, ax = plt.subplots(figsize=(9, 5.5))
labels = ["Mod1", "New"]
dt.plot_bars([sfo1.data, new.data], labels, ax=ax)

plt.show()
