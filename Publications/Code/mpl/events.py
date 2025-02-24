import matplotlib as mpl
import matplotlib.pyplot as plt
import cmcrameri.cm as cmaps
import numpy as np


def parse(file: str) -> np.ndarray:
    raw = np.loadtxt(file)
    ret = np.zeros((128, 128))
    for x, y, q in raw:
        ret[int(x), int(y)] += q
    return ret



deltas = parse("./Events/run_157_entry_46581.dat")
fig = plt.figure(figsize=(10,7))
plt.imshow(deltas.T, origin="lower", cmap=cmaps.managua_r)
plt.colorbar()
plt.show()
