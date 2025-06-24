import pyphysics as phys
import pickle
import matplotlib.pyplot as plt

# Read
data: phys.SMDataDict = {}
with open("./Outputs/18O_dt.pkl", "rb") as f:
    data = pickle.load(f)

# Quantum numbers
qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)

# Barager calculus
bar = phys.Barager()
# Removal
snrem = phys.Particle("18O").get_sn()
bar.set_removal(data, snrem)
bar.do_for([qp12, qp32])

print("SO gap in 18O : ", bar.get_gap(qp12, qp32))