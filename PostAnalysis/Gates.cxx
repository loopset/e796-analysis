#ifndef Gates_cxx
#define Gates_cxx

#include "ActMergerData.h"

#include "/media/Data/E796v2/Selector/Selector.h"

namespace E796Gates
{
auto rp {[](double xrp) -> bool
         { return (gSelector->GetRPxLow() <= xrp) && (xrp <= gSelector->GetRPxUp()); }}; // same as Juan

auto maskelsil {[](int idx) -> bool
                {
                    if(gSelector->GetMaskElSil() && (idx == 0 || idx == 3 || idx == 6))
                        return false;
                    else
                        return true;
                }};

auto masktranssil {[](int idx)
                   {
                       auto opt {gSelector->GetMaskTransSilOpt()};
                       if(opt == "" || opt == "all")
                           return true;
                       else if(opt == "center")
                       {
                           if(idx == 3 || idx == 4)
                               return true;
                           else
                               return false;
                       }
                       else if(opt == "up")
                       {
                           if(idx == 8 || idx == 8 || idx == 10)
                               return true;
                           else
                               return false;
                       }
                       else
                           return false;
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
