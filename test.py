import pyphysics as phys
import matplotlib.pyplot as plt

a = phys.Kinematics("20O(d,t)@700")
b = phys.Kinematics("20O(d,t)@700|14")

fig, ax = plt.subplots(1, 1)

for k in [a, b]:
    x, y = k.get_line3()
    ax.plot(x, y, label = f"Ex = {k.Ex}")
ax.legend()
plt.show()
