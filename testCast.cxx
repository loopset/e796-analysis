#include "ActTPCDetector.h"

#include "TStopwatch.h"
#include "TRandom.h"

#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"

#include <iostream>
#include <vector>

typedef ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<int>, ROOT::Math::DefaultCoordinateSystemTag> XYZPointI;
using XYZPoint = ROOT::Math::XYZPoint;

void Print(const ROOT::Math::XYZPoint& p)
{
    std::cout << "Point = " << p << '\n';
}

void Cast(const ROOT::Math::XYZPoint& p)
{
    ;
}
void NotCast(const XYZPointI& p)
{
    ;
}

double gMin {128 * 128};
double gMax {};
std::vector<int> gVector;

void FillWithRange(double val)
{
    if(val > gMax)
        gMax = val;
    if(val < gMin)
        gMin = val;
    gVector.push_back(val);
}

void testCast()
{
    // XYZPointI p {1, 2, 3};
    //
    // TStopwatch timer;
    // timer.Start();
    // for(int i = 0; i < (int)1e9; i++)
    // {
    //     Cast(static_cast<XYZPoint>(p));
    //     //NotCast(p);
    // }
    // timer.Stop();
    // timer.Print();
    //
    // ActRoot::TPCParameters tpc {"Actar"};
    // tpc.SetREBINZ(4);
    //
    // TStopwatch timer {};
    // timer.Start();
    // for(int i = 0; i < (int)1e9; i++)
    // {
    //     tpc.GetREBINZ();
    // }
    // timer.Stop();
    // timer.Print();

    TStopwatch timer {}; timer.Start();
    for(int i = 0; i < (int)1e9; i++)
    {
        auto rand {gRandom->Uniform()};
        auto j {(int)rand};
    }
    timer.Stop();
    timer.Print();
    //
    // for(int i = 4; i < 87; i++)
    //     FillWithRange(i);
    //
    // std::cout << "Min : " << gMin << '\n';
    // std::cout << "Max : " << gMax << '\n';
}
