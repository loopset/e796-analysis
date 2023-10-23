#include "ActGeometry.h"

#include <map>
#include <string>
#include <utility>

void BuildGeometry(bool draw = true)
{
    //Define parameters
    //Remember that we work with HALF LENGTHS
    //Drift cage for ACTAR
    double driftX { 25.6 / 2};//cm
    double driftY {  25.6 / 2};
    double driftZ { 25.6 / 2};
    ActSim::DriftChamber actar(driftX, driftY, driftZ);
    //unit silicon size
    double silicon1X { 5.0E-2 / 2};//cm
    double silicon1Y { 8.    / 2};
    double silicon1Z { 5.0    / 2};
    ActSim::SilUnit silUnit(0, silicon1X, silicon1Y, silicon1Z);
    //set placements for front L0
    std::map<int, std::pair<double, double>> l0Placements
        {
            {0, {+2 * silicon1Y, -2 * silicon1Z}},
            {1, {0, -2 * silicon1Z}},
            {2, {-2 * silicon1Y, -2 * silicon1Z}},
            {3, {+1 * silicon1Y + 2.2, 0}},
            {4, {- 1 * silicon1Y - 2.2, 0}},
            {5, {+2 * silicon1Y, + 2 * silicon1Z}},
            {6, {0, + 2 * silicon1Z}},
            {7, {- 2 * silicon1Y, + 2 * silicon1Z}},
            {8, {+ 2 * silicon1Y, + 4 * silicon1Z}},
            {9, {0, + 4 * silicon1Z}},
            {10, {- 2 * silicon1Y, + 4 * silicon1Z}}
        };
    ActSim::SilAssembly l0Assembly(0, silUnit, true, false);
    //offset from flange of ACTAR
    double l0offset {10.4};//cm
    l0Assembly.SetOffsets(l0offset);
    l0Assembly.SetAssemblyPlacements(l0Placements);
    //L1 assembly! same as L0 but only changing placements for sils 3 and 4
    auto l1Placements {l0Placements};
    l1Placements.at(3) = {-2 * silicon1Y, 0};
    l1Placements.at(4) = {+ 2 * silicon1Y, 0};
    ActSim::SilAssembly l1Assembly(1, silUnit, true, false);
    l1Assembly.SetAssemblyPlacements(l1Placements);
    l1Assembly.SetOffsets(l0offset + 2.9);

    //BUILD GEOMETRY
    ActSim::Geometry geo { };
    geo.SetDrift(actar);
    geo.AddAssemblyData(l0Assembly);
    geo.AddAssemblyData(l1Assembly);
    geo.Construct();
    geo.Print();

    //SAVE GEO
    std::string path {"./Geometries/"};
    geo.WriteGeometry(path, "geo0");

    //and draw it if necessary
    if(draw) geo.Draw();
}
