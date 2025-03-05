from matplotlib import scale
import uncertainties as unc
from fractions import Fraction
import matplotlib.pyplot as plt
import matplotlib as mpl
import re


class QuantumNumbers:
    """
    A class representing the (nlj)
    quantum numbers that identify a state
    """

    letters = {0: "s", 1: "p", 2: "d", 3: "f", 4: "g", 5: "h", 6: "i"}

    def __init__(self, n: int, l: int, j: float) -> None:
        self.n = n
        self.l = l
        self.j = j
        return

    @classmethod
    def from_str(cls, string: str):
        letter = re.search(r"[spdfghi]", string)
        if letter:
            it = letter.start()
            n = int(string[:it])
            l = -1
            for i, val in cls.letters.items():
                if val == string[it]:
                    l = i
            if l == -1:
                raise ValueError(
                    "Cannot parse string as QuantumNumber. Check the given letter"
                )
            j = float(Fraction(string[it + 1 :]))
        return cls(n, l, j)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, QuantumNumbers):
            return NotImplemented
        return self.n == other.n and self.l == other.l and self.j == other.j

    def __hash__(self) -> int:
        return hash((self.n, self.l, self.j))

    def __str__(self) -> str:
        return f"Quantum number:\n n : {self.n}\n l : {self.l}\n j : {self.j}"

    def format(self) -> str:
        frac = Fraction(self.j).limit_denominator()
        ret = rf"{self.n}{QuantumNumbers.letters[self.l]}$_{{{frac}}}$"
        return ret

    def degeneracy(self) -> int:
        return int(2 * self.j + 1)


class ShellModelData:
    """
    A class containing the Ex and SF data from a shell-model calculation
    """

    def __init__(self, ex: float | unc.UFloat, sf: float | unc.UFloat) -> None:
        self.Ex = ex
        self.SF = sf
        return

    def __str__(self) -> str:
        return f"Data:\n  Ex : {self.Ex}\n  SF : {self.SF}"


class ShellModel:
    def __init__(self, files: list = []) -> None:
        self.data: dict = {}

        if len(files):
            self.__buildFromFiles(files)
        return

    def __buildFromFiles(self, files: list) -> None:
        # Parse each file
        for file in files:
            input = self.__parse(file)
            self.data.update(input)

        # Determine binding energy
        maxSF = max(
            [s for sublist in self.data.values() for s in sublist], key=lambda sm: sm.SF
        )
        # print(maxSF)
        self.BE = maxSF.Ex
        # And substract it from states
        for _, sublist in self.data.items():
            for state in sublist:
                state.Ex = state.Ex - self.BE
        return

    def __parse(self, file: str) -> dict:
        ret = {}
        with open(file, "r") as f:
            n, l, j = -1, -1, -1
            for lin in f:
                line = lin.strip()
                if not line:
                    continue
                if "orbit" in line:
                    # Set nlj of current states
                    for c, column in enumerate(line.split()):
                        if c == 2:
                            n = int(column)
                        elif c == 3:
                            l = int(column)
                        elif c == 4:
                            j = int(column)
                if "0(" in line:
                    ex = float(line[35:41].strip())
                    c2s = float(line[45:51].strip())
                    # Define key
                    q = QuantumNumbers(n, l, j / 2)
                    # Define values
                    sm = ShellModelData(ex, c2s)
                    # Push to dict
                    if q not in ret:
                        ret[q] = [sm]
                    else:
                        ret[q].append(sm)
        return ret

    def set_max_Ex(self, maxEx: float) -> None:
        for key, vals in self.data.items():
            newlist = []
            for val in vals:
                if val.Ex <= maxEx:
                    newlist.append(val)
            self.data[key] = newlist
        return

    def set_min_SF(self, minSF: float) -> None:
        for key, vals in self.data.items():
            newlist = []
            for val in vals:
                if val.SF >= minSF:
                    newlist.append(val)
            self.data[key] = newlist
        return

    def print(self) -> None:
        print("-- Shell Model --")
        for key, vals in self.data.items():
            print(key)
            for val in vals:
                print(val)
            print("---------------")
        return


class SMOrbital:
    def __init__(self, q: QuantumNumbers):
        self.q = q
        self.n = self.get_max_occupancy()
        return

    def get_max_occupancy(self) -> int:
        return 2 * self.q.j + 1


class SMPlotter:
    shells = [
        QuantumNumbers(0, 0, 1 / 2),
        QuantumNumbers(0, 1, 3 / 2),
        QuantumNumbers(0, 1, 1 / 2),
        QuantumNumbers(0, 2, 5 / 2),
        QuantumNumbers(1, 0, 1 / 2),
        QuantumNumbers(0, 2, 3 / 2),
        QuantumNumbers(0, 3, 7 / 2),
        QuantumNumbers(1, 1, 3 / 2),
        QuantumNumbers(0, 3, 5 / 2),
        QuantumNumbers(1, 1, 1 / 2),
        QuantumNumbers(0, 4, 9 / 2),
    ]
    magic = [2, 8, 20, 50, 82, 126]

    def __init__(self, which: str, debug: bool = False, max: int = 16):
        # Assign which
        if which not in ["b", "both", "p", "proton", "n", "neutron"]:
            raise ValueError("Which is not listed in class")
        self.which = which
        self.max = max
        self.ax = None
        self.debug = debug
        self.fillings = {}
        # Default fillings
        for w in ["p", "n"]:
            self.set_fillings(w, {})
        return

    def set_fillings(self, which: str, dictionary: dict) -> None:
        if which not in ["p", "n"]:
            raise ValueError("You must pass p or n at a time")
        # Init fillings
        self.fillings[which] = []
        # Redefine dict to QuantumNumber
        input = {}
        for key, val in dictionary.items():
            input[QuantumNumbers.from_str(key)] = val
        # Check which orbitals are fully filled
        in_dict = []
        for i, q in enumerate(SMPlotter.shells):
            if q in input:
                in_dict.append(i)
        # Up to here all are filled
        all_filled = min(in_dict) - 1 if len(in_dict) else -1
        for i, q in enumerate(SMPlotter.shells):
            if i <= all_filled:
                self.fillings[which].append(q.degeneracy())
            else:
                if q in input:
                    val = input[q]
                    deg = q.degeneracy()
                    self.fillings[which].append(val if val <= deg else deg)
                else:
                    self.fillings[which].append(0)
        return

    def plot(self, ax=None, label: bool = False) -> plt.Axes:
        if ax is None:
            fig, ax = plt.subplots()
        self.ax = ax
        # Orbital width
        lmin = 0.15
        lmax = 0.75
        # Max shell to be plotted
        imax = self.get_idx_max_shell()
        # print("imax : ", imax)
        if imax > len(SMPlotter.shells) or imax == -1:
            raise IndexError("No specified shells above that")
        # Magic numbers to plot
        magics = self.get_magics()
        # Draw protons or neutrons or both
        xscales = []
        whiches = []
        if "b" in self.which:
            xscales = [1, -1]
            whiches = ["n", "p"]
        elif "n" in self.which:
            xscales = [1]
            whiches = ["n"]
        elif "p" in self.which:
            xscales = [-1]
            whiches = ["p"]
        # Get y scaling
        ylabel = int(label) * 0.15
        ypad = 0.25
        yscale = (imax - 1) + 0.5 * len(magics) + ypad * 2 + ylabel
        yscale /= len(xscales)
        # Draw!
        for which, sca in zip(whiches, xscales):
            # X positions
            xs = list(map(lambda x: x * sca, [lmin, lmax]))
            # Track magic numbers
            deg: int = 0
            y = (ypad + ylabel) / yscale
            yprev = y
            # Coords for magics
            xmagics = []
            ymagics = []
            for i in range(imax):
                q = SMPlotter.shells[i]
                deg += q.degeneracy()
                ax.plot(
                    xs,
                    [y, y],
                    color="black",
                    lw=1.5,
                )
                ax.annotate(
                    q.format(),
                    xy=(sca * (lmax + 0.015), y),
                    ha="left" if sca > 0 else "right",
                    va="center",
                    fontsize=14,
                )
                # print("i : ", i, " sum : ", deg, " y : ", y)
                # Save legacy value
                yprev = y
                # Prepare for next iteration
                y += 1 / yscale
                if deg in magics:
                    y += 0.5 / yscale
                    # print("  adding magic")
                    xmagics.append(sum(xs) / 2)
                    ymagics.append((y + yprev) / 2)
                # Draw nucleons
                self.plot_nucleons(which, xs, yprev, q)
            # Draw magic numbers
            self.plot_magics(xmagics, ymagics)
            if label:
                self.plot_label(which, xs)

        # Limits
        xlim = []
        if "b" in self.which:
            xlim = [-1, 1]
        elif "n" in self.which:
            xlim = [0, 1]
        elif "p" in self.which:
            xlim = [-1, 0]
        ax.set_xlim(xlim)
        ax.set_ylim(0, len(xscales))
        # ax.set_aspect("auto", adjustable="box")
        ax.set_aspect("equal")
        if self.debug:
            ax.grid(True, "both")
        else:
            ax.set_axis_off()

        plt.tight_layout()
        return self.ax  # or ax, is the same

    def plot_magics(self, xs: list, ys: list) -> None:
        for i, coords in enumerate(zip(xs, ys)):
            x, y = coords
            circle = mpl.patches.Circle((x, y), 0.1, color="none", ec="black")
            self.ax.add_patch(circle)
            self.ax.annotate(
                rf"{SMPlotter.magic[i]}",
                xy=(x, y),
                ha="center",
                va="center",
                fontsize=16,
            )
        return

    def plot_nucleons(self, which: str, x: list, y: float, q: QuantumNumbers) -> None:
        # Determine how many nucleons
        idx = SMPlotter.shells.index(q)
        n = self.fillings[which][idx]
        # Variables of plotting on the line
        xmin = min(x)
        xmax = max(x)
        radius = 0.03
        deg = q.degeneracy()
        length = abs(abs(xmax) - abs(xmin))
        offset = 0.0
        needed = length - 2 * offset
        step = needed / deg
        p = xmin + offset + step / 2
        # Color depending on particle
        color = "dodgerblue" if which == "n" else "crimson"
        for _ in range(n):
            c = mpl.patches.Circle((p, y), radius, color=color, zorder=4)
            p += step
            self.ax.add_patch(c)

        return

    def plot_label(self, which: str, x: list) -> None:
        meanx = sum(x) / 2
        self.ax.annotate(
            "Protons" if which == "p" else "Neutrons",
            xy=(meanx, 0),
            ha="center",
            va="center",
            fontsize=18,
        )
        return

    def get_idx_max_shell(self) -> int:
        sum = 0
        for i, s in enumerate(SMPlotter.shells):
            sum += s.degeneracy()
            if sum > self.max:
                return i
        return -1

    def get_magics(self) -> list:
        ret = []
        for m in SMPlotter.magic:
            if m < self.max:
                ret.append(m)
        return ret
