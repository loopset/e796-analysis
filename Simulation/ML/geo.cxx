#include "ActSilSpecs.h"
#include "ActTPCParameters.h"
void geo()
{
    ActRoot::TPCParameters tpc {"Actar"};

    ActPhysics::SilSpecs sils;
    sils.ReadFile("./Inputs/silicons.conf");
    // Center silicons on beam
    sils.GetLayer("f0").MoveZTo(tpc.Z() / 2, {7});
    sils.GetLayer("f1").MoveZTo(tpc.Z() / 2, {7});
    sils.DrawGeo();
}
