#include "ActDataManager.h"
#include "ActKinematics.h"
#include "ActMergerData.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TMath.h"

#include <algorithm>
#include <cmath>
#include <vector>
void test()
{
    ActRoot::DataManager dataman {"./configs/data.conf"};
    dataman.SetRuns(155, 156);
    auto chain {dataman.GetChain()};
    auto chain2 {dataman.GetChain(ActRoot::ModeType::EFilter)};
    chain->AddFriend(chain2.get());

    ROOT::RDataFrame df {*chain};
    auto gated {df.Filter("fLightIdx != -1")};
    auto def {gated.Define("ZsSize",
                           [](ActRoot::TPCData& tpc, ActRoot::MergerData& merger)
                           {
                               std::vector<int> sizes;
                               for(const auto& voxel : tpc.fClusters[merger.fLightIdx].GetRefToVoxels())
                               {
                                   sizes.push_back(voxel.GetZs().size());
                               }
                               auto min = *std::max_element(sizes.begin(), sizes.end());
                               return min;
                           },
                           {"TPCData", "MergerData"})};
    auto h2d {def.Histo2D({"h2d", ";SP.Z [mm];Zs size", 300, 0, 300, 40, 0, 40}, "fSP.fCoordinates.fZ", "ZsSize")};

    h2d->DrawClone("colz");
}
