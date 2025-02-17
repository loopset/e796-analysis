class State:
    def __init__(self, ex: float, n: int, l: int, j2: int, sf: float):
        self.fEx = ex
        self.fn = n
        self.fl = l
        self.fj = j2 / 2
        self.fc2s = sf

    def print(self):
        print("---- State ----")
        print(
            "Ex : ",
            self.fEx,
            " n : ",
            self.fn,
            " l : ",
            self.fl,
            " j : ",
            self.fj,
            " C2S : ",
            self.fc2s,
        )
        print("--------------------")


def parse(file: str) -> list:
    ret = []
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
                ret.append(State(ex, n, l, j, c2s))
    return ret


def shift_Ex(states: list) -> None:
    maxC2S = max(states, key=lambda o: o.fc2s)
    shift = maxC2S.fEx
    for state in states:
        state.fEx = state.fEx - shift
