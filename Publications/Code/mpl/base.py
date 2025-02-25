import matplotlib as mpl
from mpl_toolkits.axes_grid1.inset_locator import inset_axes
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import matplotlib.pyplot as plt
plt.style.use("./actroot.mplstyle")
import numpy as np
import cmcrameri.cm as cmc
import ROOT as r

# Define function to add colorbar axes
def add_colorbar(im, ax):
    cax = inset_axes(ax, width="5%", height="100%", loc="lower left",
                     bbox_to_anchor= (1.04, 0, 1, 1), bbox_transform = ax.transAxes, 
                     borderpad = 0)
    return plt.gcf().colorbar(im, cax = cax)

# Function to force X and Y ticks to be equal
def set_ticks(ax):
    ticks = np.arange(0, 128, 20)
    ax.set(xticks=ticks, yticks=ticks)
    return

# Main fcn to plot
def plot(matrix, filename, ax = None):
    if ax is None:
        fig, ax = plt.subplots(figsize=(4,4))

    im = ax.imshow(matrix.T, origin="lower", aspect="equal", interpolation="none", 
                   cmap=cmc.managua_r)
    set_ticks(ax)
    ax.set_xlabel("X [pads]", loc="right")
    ax.set_ylabel("Y [pads]", loc="top")
    # cbar = add_colorbar(im, ax)
    # cbar.ax.locator_params(nbins=5)
    if len(filename):
        plt.savefig(f"{filename}.pdf")

def plot_unique(matrix: np.ndarray, name: str, ax: plt.Axes = None):
    if not ax:
        fig = plt.figure(name, figsize=(3.5,3.5))
        fig.clf()
        ax = fig.add_subplot(111);
    # Imshow
    im = ax.imshow(matrix.T, origin="lower", aspect="auto", interpolation="none", 
                   cmap=cmc.managua_r)
    set_ticks(ax)
    ax.set_xlabel("X [pads]", loc="right")
    ax.set_ylabel("Y [pads]", loc="top")
    # cbar = add_colorbar(im, ax)
    # cbar.ax.locator_params(nbins=5)

def format_ax(ax):
    set_ticks(ax)
    ax.set_xlabel("X [pads]", loc="right")
    ax.set_ylabel("Y [pads]", loc="top")

# Function to parse txt file
def parse(file: str, proj: str = "xy") -> np.ndarray:
    raw = np.loadtxt(file)
    if "3d" in proj:
        ret = np.zeros((128, 128, 128))
    else:
        ret = np.zeros((128, 128))
    dim = len(ret.shape)
    for x, y, z, q in raw:
        if dim == 3: 
            ret[int(x), int(y), int(z)] += q
        else:
            if "xy" in proj:
                ret[int(x), int(y)] += q
            else:
                ret[int(x), int(z)] += q
    ## Mask empty positions to Nan
    ret[ret == 0] = np.nan
    return ret

def parse_with_id(file: str, proj: str = "xy") -> tuple:
    raw = np.loadtxt(file)
    if "3d" in proj:
        ret = np.full((128, 128, 128), np.nan)
        ids = np.full((128, 128, 128), np.nan)
    else:
        ret = np.full((128, 128), np.nan)
        ids = np.full((128, 128), np.nan)
    dim = len(ret.shape)
    for x, y, z, q, id in raw:
        if dim == 3: 
            ret[int(x), int(y), int(z)] += q
            ids[int(x), int(y), int(z)]  = id
        else:
            if "xy" in proj:
                ret[int(x), int(y)] += q
                ids[int(x), int(y)] = id
            else:
                ret[int(x), int(z)] += q
                ids[int(x), int(z)] = id
    return (ret, ids)

def parse_lines(file: str) -> list:
    raw = np.loadtxt(file)
    ret = []
    for _, px, py, pz, vx, vy, vz in raw:
        line = r.ActRoot.Line(r.ROOT.Math.XYZPointF(px, py, pz), r.ROOT.Math.XYZVectorF(vx, vy, vz), 0)
        ret.append(line)
    return ret

def plot_line(l: object, xmin, xmax, ax: plt.Axes, **kwargs):
    vx = []
    vy = []
    for x in np.arange(xmin, xmax, 1):
        pos = l.MoveToX(x)
        vx.append(x)
        vy.append(pos.Y())
    return ax.plot(vx, vy, **kwargs)
        

        
