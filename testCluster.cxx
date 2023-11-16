#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3D.h"
#include "Math/Vector3Dfwd.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

using XYZPoint = ROOT::Math::XYZPoint;

void testCluster()
{

    std::vector<XYZPoint> rps {{64, 64, 64}, {66, 65, 64}, {78, 48, 64}, {76, 47, 63}};
    auto cmp {[](const XYZPoint& l, const XYZPoint& r)
              {
                  if(l.X() != r.X())
                      return l.X() < r.X();
                  if(l.Y() != r.Y())
                      return l.Y() < r.Y();
                  return l.Z() < r.Z();
              }};
    // Sort rps
    std::sort(rps.begin(), rps.end(), cmp);
    double distThresh {5};
    std::vector<std::vector<XYZPoint>> clusters;
    for(auto it = std::begin(rps);;)
    {
        auto last {std::adjacent_find(it, std::end(rps),
                                      [&](const XYZPoint& l, const XYZPoint& r) { return (l - r).R() > distThresh; })};
        if(last == std::end(rps))
        {
            clusters.emplace_back(it, last);
            break;
        }
        auto gap {std::next(last)};
        clusters.emplace_back(it, gap);

        it = gap;
    }

    // Print data
    for(const auto& rp : rps)
        std::cout << "RP : " << rp << '\n';
    // Print clusters
    for(const auto& cl : clusters)
    {
        std::cout << "Cluster with size : " << cl.size() << '\n';
        for(const auto& rp : cl)
            std::cout << "  RP in cl : " << rp << '\n';
    }
}
