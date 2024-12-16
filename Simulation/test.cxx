#include "ActSilSpecs.h"

#include "Math/Point3Dfwd.h"
#include "Math/Vector3Dfwd.h"
#include <iomanip>
#include <iostream>

void test()
{
    ActPhysics::SilSpecs specs {};
    specs.ReadFile("../configs/detailedSilicons.conf");

    std::cout<<std::setprecision(8);
    ROOT::Math::XYZPoint p {128., 128., 128};
    ROOT::Math::XYZVector v {0.05, 0.95, 0};
    specs.FindLayerAndIdx(p, v, true);
}
