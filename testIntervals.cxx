#include <iostream>

#include "ActInterval.h"

void testIntervals()
{
    ActCluster::Interval a {81, 118};
    ActCluster::Interval b {68, 80};
    ActCluster::Interval c {5, 5};

    std::cout << "A = " << a << '\n';
    std::cout << "B = " << b << '\n';

    std::cout << "Overlaps = " << a.Overlaps(b) << '\n';
    std::cout << "Intersect = " << a.Intersect(b) << '\n';
    std::cout << "A < B ? " << (a < b) << '\n';
    std::cout << "A <= B ? " << (a <= b) << '\n';
    std::cout << "A > B ? " << (a > b) << '\n';
    std::cout << "A >= B ? " << (a >= b) << '\n';
    std::cout << "A.Dist(B) = " << a.Distance(b) << '\n';
    ////
    std::set<int> set {60, 62, 63};

    ActCluster::IntervalMap<int> im;
    im.BuildFromSet(0, set);
    // im.Add(0, c);
    std::cout << "Map.size() = " << im.GetSizeInKey(0) << '\n';
    std::cout << "Max distance = " << im.GetMaximumDistance(0) << '\n';
    im.Print();
}
