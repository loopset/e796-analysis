from scipy.interpolate import CubicSpline
import numpy as np


def create_spline3(x, y) -> CubicSpline:
    return CubicSpline(x, y)
