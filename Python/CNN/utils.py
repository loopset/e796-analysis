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
    labels = np.array(dictionary["label"])
    return (data, labels)

def TransformData(data : np.ndarray, grid_size : int ) -> np.ndarray:
    ret = np.zeros((len(data), grid_size, grid_size, 1), dtype=np.float32)
    for i in range(len(data)):
        for j in range(len(data[i])):
            x = data[i][j][0]
            y = data[i][j][1]
            q = data[i][j][2]
            ret[i, x, y, 0] += q
    return ret