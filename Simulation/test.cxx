#include "ActSilMatrix.h"
#include "ActSilSpecs.h"

#include "TAttLine.h"

#include "Math/Point3Dfwd.h"
#include "Math/Vector3Dfwd.h"

#include "../PostAnalysis/Utils.cxx"
void test()
{
    ActPhysics::SilSpecs specs;
    specs.ReadFile("../configs/detailedSilicons.conf");
    specs.DrawGeo();

    ROOT::Math::XYZPoint vertex {128, 128, 50};
    ROOT::Math::XYZVector dir {0.1, +0.9, 0};
    auto [i, sp] = specs.FindSPInLayer("l0", vertex, dir);
    std::cout << "i : " << i << " SP : " << sp << '\n';
}
