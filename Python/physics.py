from matplotlib.pyplot import sca
import shell_model as sm

import uncertainties as unc

from typing import Dict, List


class BaragerRes:
    def __init__(self) -> None:
        self.NumRem: float | unc.UFloat = 0
        self.NumAdd: float | unc.UFloat = 0
        self.DenRem: float | unc.UFloat = 0
        self.DenAdd: float | unc.UFloat = 0
        self.ESPE: float | unc.UFloat = 0

        return

    def __str__(self) -> str:
        return f"-- BaragerRes --\n N-   : {self.NumRem}\n N+   : {self.NumAdd}\n D-   : {self.DenRem}\n D+   : {self.DenAdd}\n ESPE : {self.ESPE}"

    def do_adding(
        self, q: sm.QuantumNumbers, add: sm.ShellModel, sn: float, scale: float = 1
    ) -> None:
        if q in add.data:
            for state in add.data[q]:
                # Numerator
                self.NumAdd += (2 * q.j + 1) * (scale * state.SF) * (state.Ex - sn)
                # Denominator
                self.DenAdd += (2 * q.j + 1) * (scale * state.SF)
        return

    def do_removal(
        self, q: sm.QuantumNumbers, rem: sm.ShellModel, sn: float, scale: float = 1
    ) -> None:
        if q in rem.data:
            for state in rem.data[q]:
                self.NumRem += (scale * state.SF) * (-sn - state.Ex)
                self.DenRem += scale * state.SF
        return

    def do_espe(self) -> None:
        try:
            self.ESPE = (self.NumAdd + self.NumRem) / (self.DenAdd + self.DenRem)
        except ZeroDivisionError:
            print("Barager:do_spe() got a zero in denominator. Check your inputs")
        return


class Barager:
    def __init__(self) -> None:
        self.Rem: sm.ShellModel | None = None
        self.Add: sm.ShellModel | None = None
        self.SnRem: float = 0
        self.SnAdd: float = 0
        self.ScaleAdd: float = 1
        self.ScaleRem: float = 1
        self.Results: Dict[sm.QuantumNumbers, BaragerRes] = {}
        return

    def set_removal(self, rem: sm.ShellModel, sn: float, scale: float = 1) -> None:
        self.Rem = rem
        self.SnRem = sn
        self.ScaleRem = scale
        return

    def set_adding(self, add: sm.ShellModel, sn: float, scale: float = 1) -> None:
        self.Add = add
        self.SnAdd = sn
        self.ScaleAdd = scale
        return

    def do_for(self, qs: List[sm.QuantumNumbers]) -> None:
        for q in qs:
            res = BaragerRes()
            # Removal
            if self.Rem is not None:
                res.do_removal(q, self.Rem, self.SnRem, self.ScaleRem)
            # Adding
            if self.Add is not None:
                res.do_adding(q, self.Add, self.SnAdd, self.ScaleAdd)
            res.do_espe()
            # Results
            self.Results[q] = res
        return

    def get_gap(
        self, q0: sm.QuantumNumbers, q1: sm.QuantumNumbers
    ) -> float | unc.UFloat:
        if q0 in self.Results and q1 in self.Results:
            return abs(self.Results[q0].ESPE - self.Results[q1].ESPE)
        return 0

    def print(self) -> None:
        print("----- Barager -----")
        for q, val in self.Results.items():
            print(q)
            print(val)
        print("--------------------")
        return
