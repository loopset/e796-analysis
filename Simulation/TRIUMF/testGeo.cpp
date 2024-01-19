#include "ActGeometry.h"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "Math/Vector3Dfwd.h"

void testGeo() 
{
    ActSim::Geometry geo {};
    geo.ReadGeometry("./Geometries/", "triumf");

    ROOT::Math::XYZPoint vertex {10.14, 0.55, -4.29};
    ROOT::Math::XYZVector dir {-0.99, -0.12, 0.07};

    ROOT::Math::XYZPoint silPoint0 {};
    int silType0 {};
    int silIndex0 {};
    double distance0 {};
    bool side0 {};
    // Assembly 0
    int hitAssembly0 {};
    geo.PropagateTrackToSiliconArray(vertex, dir, 2, side0, distance0, silType0, silIndex0, silPoint0, true);

}
