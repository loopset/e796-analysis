#ifndef Gates_cxx
#define Gates_cxx

#include "ActMergerData.h"
namespace E796Gates
{
    auto left0 {[](const ActRoot::MergerData& d)
                {
                    // Check size
                    bool hasSize {d.fSilLayers.size() == 1};
                    if(!hasSize)
                        return hasSize;
                    bool isL0 {d.fSilLayers.front() == "l0"};
                    return isL0;
                }};

    auto front0 {[](const ActRoot::MergerData& d)
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
