file="/home/miguel/Descargas/ScientificColourMaps8/managua/managua.clm"

cm = []
with open(file, "r") as f:
    lines = f.readlines()[::-1]
    for i, line in enumerate(lines):
        # Separate colors
        val = f"rgb255({i})=("
        for color in line.split():
            val += f"{color},"
        val = val[:-1]
        val += ");"
        print(val)
