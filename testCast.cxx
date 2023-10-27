#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"
#include "TStopwatch.h"

#include <iostream>

typedef ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<int>, ROOT::Math::DefaultCoordinateSystemTag> XYZPointI;
using XYZPoint = ROOT::Math::XYZPoint;

void Print(const ROOT::Math::XYZPoint& p)
{
    std::cout<<"Point = "<<p<<'\n';
}

void Cast(const ROOT::Math::XYZPoint& p){;}
void NotCast(const XYZPointI& p) {;}

void testCast()
{
    XYZPointI p {1, 2, 3};

    TStopwatch timer;
    timer.Start();
    for(int i = 0; i < (int)1e9; i++)
    {
        Cast(static_cast<XYZPoint>(p));
        //NotCast(p);
    }
    timer.Stop();
    timer.Print();
}
