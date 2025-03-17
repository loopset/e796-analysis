class Particle:
    def __init__(self, z: int, a: int):
        self.fA: int = a
        self.fZ: int = z
        self.fN: int = self.fA - self.fZ
        return

    def __str__(self):
        return f"-- Particle --\n  A : {self.fA}\n  Z : {self.fZ}  N : {self.fN}"
