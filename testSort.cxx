#include "ActTPCData.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
void testSort()
{
    std::vector<ActRoot::Voxel> v {ActRoot::Voxel{{5, 0, 1}, 0}, ActRoot::Voxel {{1, 0, 2}, 0}, ActRoot::Voxel {{2, 0, 8}, 0}};
    std::sort(v.begin(), v.end());
    for(const auto& e : v)
    {
        std::cout << "Using usual sort : " << e.GetPosition() << '\n';
    }
    std::sort(v.begin(), v.end(), std::greater<ActRoot::Voxel>());
    for(const auto& e : v)
    {
        std::cout << "Using greater sort : " << e.GetPosition() << '\n';
    }
}
