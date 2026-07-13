import pyphysics as phys
import numpy as np


def get_neutron_rms_be(s, p, d):
    weights = np.array([0.28, 0.87, 0.41])
    rs = np.array([s, p, d])
    a = weights**2 * rs**2
    sa = np.sqrt(np.sum(a))
    print("For rms : ", rs)
    print("rms ", sa)
    return sa


def get_neutron_rms_li(s, p, d):
    weights = np.array([0.51, 0.74, 0.43])
    rs = np.array([s, p, d])
    a = weights**2 * rs**2
    sa = np.sqrt(np.sum(a))
    print("For rms : ", rs)
    print("rms ", sa)
    return sa


def estimate_12be(nrms):
    matter_10be = 2.30
    return np.sqrt((10 * matter_10be**2 + 2 * nrms**2) / 12)


def estimate_11li(nrms):
    matter_9li = 2.34
    return np.sqrt((10 * matter_9li**2 + 2 * nrms**2) / 12)


# r0 = 1.13
i0 = get_neutron_rms_be(4.76, 3.74, 3.36)
print(estimate_12be(i0))

# r0 = 0.8
i1 = get_neutron_rms_be(4.515, 3.437, 2.955)
print(estimate_12be(i1))

# r0 = 0.5
i2 = get_neutron_rms_be(4.327, 3.201, 2.659)
print(estimate_12be(i2))


# 11LI
# r0 = 2.64 fm
ii0 = get_neutron_rms_li(11.157, 8.036, 6.591)
print(estimate_11li(ii0))

# r0 = 2 fm
ii1 = get_neutron_rms_li(10.653, 7.189, 5.393)
print(estimate_11li(ii1))

# r0 = 1.75 fm
ii2 = get_neutron_rms_li(10.462, 6.927, 4.907)
print(estimate_11li(ii2))

# r0 = 1.5 fm
ii3 = get_neutron_rms_li(10.275, 6.526, 4.495)
print(estimate_11li(ii3))

# r0 = 1.25 fm
ii4 = get_neutron_rms_li(10.093, 6.205, 4.075)
print(estimate_11li(ii4))

