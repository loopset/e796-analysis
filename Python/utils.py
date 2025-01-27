import ROOT as r
import numpy as np

def ReadROOTFile(file : str, last : int = None) -> tuple:
    rdf = r.RDataFrame("Dataset", file)
    node = rdf
    if last:
        node = node.Range(0, last)
    dictionary = node.AsNumpy(["x", "y", "z", "q", "label"])
    x = np.array(dictionary["x"])
    y = np.array(dictionary["y"])
    z = np.array(dictionary["z"])
    q = np.array(dictionary["q"])
    data = np.empty(len(x), dtype=object)
    for row in range(len(data)):
        data[row] = np.column_stack((x[row], y[row], q[row]))
    # data = np.column_stack((x, y, z, q))
    # data = np.empty((len(x), 4), dtype=object)
    # data[:, 0] = x
    # data[:, 1] = y
    # data[:, 2] = z
    # data[:, 3] = q
    labels = np.array(dictionary["label"])
    return (data, labels)

data, labels = ReadROOTFile('dataset.root')

