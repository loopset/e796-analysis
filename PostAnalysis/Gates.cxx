#ifndef Gates_cxx
#define Gates_cxx

#include "ActMergerData.h"

#include "Math/GenVector/Cartesian3D.h"
#include "Math/GenVector/PositionVector3D.h"
#include "Math/Point3Dfwd.h"
namespace E796Gates
{
auto rp {[](double xrp) -> bool { return (26 <= xrp) && (xrp <= 220); }};

auto rpMerger {[](const ActRoot::MergerData& d) -> bool { return rp(d.fRP.X()); }};

template <typename T = float>
auto rpx1 {[](const ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<T>>& rp) -> bool { return rp.X() < 128; }};

template <typename T = float>
auto rpx2 {[](const ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<T>>& rp) -> bool { return rp.X() > 128; }};

auto left0 {[](const ActRoot::MergerData& d) -> bool
            {
                // Check size
                bool hasSize {d.fSilLayers.size() == 1};
                if(!hasSize)
                    return hasSize;
                bool isL0 {d.fSilLayers.front() == "l0"};
                return isL0;
            }};

auto front0 {[](const ActRoot::MergerData& d) -> bool
             {
                 // Check size
                 bool hasSize {d.fSilLayers.size() == 1};
                 if(!hasSize)
                     return hasSize;
                 bool isF0 {d.fSilLayers.front() == "f0"};
                 return isF0;
             }};
} // namespace E796Gates

#endif // !Gates_cxx
