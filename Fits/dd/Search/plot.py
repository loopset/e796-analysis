import pyphysics as phys
import matplotlib.pyplot as plt

g1 = phys.BetaFinder("../Outputs/xs/g1_xs.dat", "./g1_Daeh/l2", "202")
g1.plot()
print("Beta : ", g1.fBeta)

plt.show()
