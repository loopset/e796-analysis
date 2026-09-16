#include "ActDataManager.h"
#include "ActModularData.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"

#include <vector>

typedef ROOT::RVecF Vector;

void InPadPlane()
{
    // Read data
    ROOT::EnableImplicitMT();
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EFilter};
    datman.SetRuns(155, 165);
    auto chain {datman.GetChain()};
    auto chain2 {datman.GetChain(ActRoot::ModeType::EMerge)};
    auto chain3 {datman.GetChain(ActRoot::ModeType::EReadSilMod)};
    chain->AddFriend(chain2.get());
    chain->AddFriend(chain3.get());
    chain->SetBranchStatus("fClusters.fVoxels", false);
    ROOT::RDataFrame df {*chain};

    // Filter by GATCONF and only one BL in cluster vector
    auto gated {df.Filter([](ActRoot::ModularData& m) { return m.Get("GATCONF") == 1; }, {"ModularData"})
                    .Filter("fClusters.fIsBeamLike.size() == 1")
                    .Filter("fClusters.fIsBeamLike.front() == true")};

    ROOT::TThreadedObject<TH2D> hPad {"hPad", "Pad;X [pad];Y [pad]", 128, 0, 128, 128, 0, 128};
    gated.Foreach(
        [&](ActRoot::TPCData& data)
        {
            auto& voxels {data.fClusters.front().GetVoxels()};
            for(const auto& voxel : voxels)
                hPad.Get()->Fill(voxel.GetPosition().X(), voxel.GetPosition().Y());
        },
        {"TPCData"});

    // Plot
    auto* c0 {new TCanvas {"c0", "In pad plane"}};
    hPad.Merge()->DrawClone("colz");
}
