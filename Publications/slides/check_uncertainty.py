import pyphysics as phys
import matplotlib.pyplot as plt
import uncertainties as un
import numpy as np
import hist
from lmfit.models import GaussianModel

import sys

sys.path.append("../")
sys.path.append("/media/Data/E796v2/Publications/dt/")

import styling as sty
import dt

qp12 = phys.QuantumNumbers(0, 1, 0.5)
qp32 = phys.QuantumNumbers(0, 1, 1.5)

sm = dt.build_sm(withSys=True)  # type: ignore

b = phys.Barager()
b.set_adding({}, 0)
b.set_removal(sm, 0)
b.do_for([qp12])
b.print()

exs = [un.ufloat(3.25, 0.02), un.ufloat(4.73, 0.05), un.ufloat(15.04, 0.04)]
c2s = [un.ufloat(0.70, 0.12), un.ufloat(0.46, 0.08), un.ufloat(0.38, 0.08)]


# Baranger formula:
def do_barager(exs, c2s):
    num = sum([c * ex for c, ex in zip(c2s, exs)])  # type: ignore
    denom = sum([c for c in c2s])  # type: ignore
    ratio = num / denom
    return ratio, num, denom


ratio, num, denom = do_barager(exs, c2s)
# Manual uncertainty
manual = np.sqrt(
    (1.0 / denom.n) ** 2 * (num.s) ** 2 + (-num.n / denom.n**2) ** 2 * (denom.s) ** 2  # type: ignore
)
print(f"Num : {num}")
print(f"Den : {denom}")
print(f"Ratio : {ratio:.2uS}")
print(f"Manual : {manual}")


h = hist.Hist.new.Reg(200, 4, 9, label="ESPE").Double()

niter = 50000
for i in range(niter):
    exi = []
    for ex in exs:
        exi.append(np.random.normal(loc=ex.n, scale=ex.s))
    ci = []
    for c in c2s:
        ci.append(np.random.normal(loc=c.n, scale=c.s))
    ratio, _, _ = do_barager(exi, ci)
    h.fill(un.nominal_value(ratio))

fig, ax = plt.subplots(1, 1)
h.plot(ax=ax)

# Extract histogram data for fitting
bins = h.axes[0]
bin_edges = bins.edges
bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
counts = np.array(h.values())

# Fit to Gaussian
model = GaussianModel()
params = model.guess(counts, x=bin_centers)
result = model.fit(counts, params, x=bin_centers)

# Plot fit
x_fit = np.linspace(bin_centers.min(), bin_centers.max(), 300)
y_fit = result.eval(x=x_fit)
ax.plot(x_fit, y_fit, 'r-', linewidth=2, label='Gaussian fit')

print(result.fit_report())
ax.legend()
plt.show()
