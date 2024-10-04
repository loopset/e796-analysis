#ifndef Gates_cxx
#define Gates_cxx

#include "ActMergerData.h"

#include "/media/Data/E796v2/Selector/Selector.h"

namespace E796Gates
{
// auto rp {[](double xrp) -> bool { return (26 <= xrp) && (xrp <= 220); }};
auto rp {[](double xrp) -> bool
         { return (gSelector->GetRPxLow() <= xrp) && (xrp <= gSelector->GetRPxUp()); }}; // same as Juan

auto maskelsil {[](int idx) -> bool
                {
                    if(gSelector->GetMaskElSil() && (idx == 0 || idx == 3 || idx == 6))
                        return false;
                    else
                        return true;
                }};

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
