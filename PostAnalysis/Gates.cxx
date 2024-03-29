#ifndef Gates_cxx
#define Gates_cxx

#include "ActMergerData.h"

#include "Math/Point3Dfwd.h"
namespace E796Gates
{
auto rp {[](double xrp) -> bool { return (26 <= xrp) && (xrp <= 220); }};

auto rpMerger {[](const ActRoot::MergerData& d) -> bool { return rp(d.fRP.X()); }};

auto rpx1 {[](const ROOT::Math::XYZPointF& rp) -> bool { return rp.X() < 128; }};

auto rpx2 {[](const ROOT::Math::XYZPointF& rp) -> bool { return rp.X() > 128; }};

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
