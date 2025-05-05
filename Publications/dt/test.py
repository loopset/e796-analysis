import ROOT as r  # type: ignore
from scipy.stats import norm
from scipy.special import voigt_profile
import numpy as np
import matplotlib.pyplot as plt

amp = 200
mean = 5
sigma = 0.2
lg = 0.5

x = np.linspace(0, 10, 400)

# root
rg = [amp * r.TMath.Gaus(e, mean, sigma) for e in x]  # type: ignore
rv = [amp * r.TMath.Voigt(e - mean, sigma, lg) for e in x]  # type: ignore
# scipy
sg = [amp * np.sqrt(2 * np.pi) * sigma * norm.pdf(e, loc=mean, scale=sigma) for e in x]
sv = [amp * 2 * voigt_profile(e - mean, sigma, lg) for e in x]

plt.subplot(121)
plt.plot(x, rg, label="ROOT")
plt.plot(x, sg, label="Scipy")
plt.legend()
plt.subplot(122)
plt.plot(x, rv, label="Voigt ROOT")
plt.plot(x, sv, label="Voigt Scipy")
plt.legend()

plt.tight_layout()
plt.show()
