import uncertainties as unc

class QuantumNumber:
    """
    A class representing the (nlj)
    quantum numbers that identify a state
    """

    def __init__(self, n: int, l: int, j: float) -> None:
        self.n = n
        self.l = l
        self.j = j
        return

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, QuantumNumber):
            return NotImplemented
        return self.n == other.n and self.l == other.l and self.j == other.j

    def __hash__(self) -> int:
        return hash((self.n, self.l, self.j))

    def __str__(self) -> str:
        return f"Quantum number:\n n : {self.n}\n l : {self.l}\n j : {self.j}"


class ShellModel:
    """
    A class containing the Ex and C2S from a shell-model calculation
    """

    def __init__(self, ex: float | unc.UFloat, sf: float | unc.UFloat) -> None:
        self.Ex = ex
        self.SF = sf
        return

    def __str__(self) -> str:
        return f"Shell Model:\n Ex : {self.Ex}\n SF : {self.SF}"


class BaragerRes:
    def __init__(
        self,
        addStr: float | unc.UFloat,
        remStr: float | unc.UFloat,
        espe: float | unc.UFloat,
    ) -> None:
        self.AddStr = addStr
        self.RemStr = remStr
        self.ESPE = espe
        return


def parse(file: str) -> dict:
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
                q = QuantumNumber(n, l, j / 2)
                # Define values
                sm = ShellModel(ex, c2s)
                # Push to dict
                if q not in ret:
                    ret[q] = [sm]
                else:
                    ret[q].append(sm)
    return ret


def shift_Ex(states: dict) -> None:
    maxC2S = max(
        [state for sublist in states.values() for state in sublist], key=lambda o: o.SF
    )
    print(maxC2S)
    shift = maxC2S.Ex
    for _, sublist in states.items():
        for state in sublist:
            state.Ex = state.Ex - shift
    return


def read_theo(fp: str, fn: str) -> dict:
    pos = parse(fp)
    neg = parse(fn)
    merged = {**pos, **neg}
    shift_Ex(merged)
    return merged
