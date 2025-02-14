# pyright: ignore[attr-defined]
import ROOT as r
import uncertainties as unc
import matplotlib.pyplot as plt

o20 = r.ActPhysics.Particle("20O")  # type: ignore[attr-defined]
o21 = r.ActPhysics.Particle("21O")  # type: ignore[attr-defined]


class SpeInfo:
    def __init__(self, ex: float | unc.UFloat, sf: float | unc.UFloat):
        self.fEx = ex
        self.fSF = sf

    def Ex(self) -> float | unc.UFloat:
        return self.fEx

    def SF(self) -> float | unc.UFloat:
        return self.fSF


# Use interface to get Ex
inter = r.Fitters.Interface()  # type: ignore[attr-defined]
inter.Read("./Outputs/interface.root", "./Outputs/fit_juan_RPx.root")
# Open the file
file = r.TFile("./Outputs/sfs.root")  # type: ignore[attr-defined]
# Experimental info
exp = {}
for state in file.Get("Keys"):
    col = file.Get(f"{state}_sfs")
    ex = unc.ufloat(inter.GetParameter(state, 1), inter.GetUnc(state, 1))
    ## Get best chi2 model
    best = col.GetBestChi2()
    sf = unc.ufloat(best.GetSF(), best.GetUSF())
    exp[state.decode("utf-8")] = SpeInfo(ex, sf)

## Map Jpis to states
map = {"5/2+": ["g0"], "1/2-": ["g2", "v0", "v1", "v4"]}

## And read adding measurements
adding = {"5/2+": [SpeInfo(unc.ufloat(0, 0), unc.ufloat(0.34, 0.03))], "1/2-": []}


def compute(J: str) -> unc.UFloat:
    # Convert to value
    expression = "".join(c for c in J if c.isdigit() or c == "/")
    j = float(eval(expression))
    # print("j : ", j)
    a = sum(
        (2 * j + 1) * info.SF() * (info.Ex() + (-1) * o21.GetSn()) for info in adding[J]
    )
    b = sum(
        info.SF() * (-1 * o20.GetSn() - info.Ex())
        for s in map[J]
        if (info := exp[s]) is not None
    )
    # print("a : ", a, " b : ", b)
    c = sum((2 * j + 1) * info.SF() for info in adding[J])
    d = sum(info.SF() for s in map[J] if (info := exp[s]) is not None)
    # print("c : ", c, " d : ", d)
    return (a + b) / (c + d)


d52 = compute("5/2+")
p12 = compute("1/2-")

print("==== Baranger formula for 20O =====")
print("==== N = 8 =====")
print(" 1d5/2 : ", d52)
print(" 1p1/2 : ", p12)
print(" Gap   : ", abs(d52 - p12))

# Plot some things
z = [6, 8, 12]
ld52 = [-0.47, -4.1, d52.n]
ed52 = [0, 0, d52.s]
lp12 = [-8.2, -15.6, p12.n]
ep12 = [0, 0, p12.s]
plt.errorbar(z, ld52, yerr=ed52, marker='o', label='1d5/2')
plt.errorbar(z, lp12, yerr=ep12, marker='o', label='1p1/2')
plt.xlabel("Z")
plt.ylabel("Binding energy [MeV]")
plt.suptitle("N = 8 shell gap")
plt.legend()
plt.show()
