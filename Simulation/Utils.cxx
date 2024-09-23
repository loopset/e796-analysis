#ifndef E796Simu_Utils_cxx
#define E796Simu_Utils_cxx

#include "Math/Point3D.h"

namespace E796Simu
{
struct Data
{
    using XYZPoint = ROOT::Math::XYZPoint;
    // Define data generated in simulation
    XYZPoint fSilPoint {};
    XYZPoint fVertex {};
    double fTBeam {};
    double fT3 {};
    double fTheta3 {};
    double fThetaCM {};
    double fWeight {};
    double fELoss0 {};
    double fELoss1 {};
    double fDistL0 {};
    double fDistInter {};
    int fSilIndex0 {};
    int fSilIndex1 {};
};
} // namespace E796Simu
#endif
