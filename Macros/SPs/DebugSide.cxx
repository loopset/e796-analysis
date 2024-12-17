#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActSilSpecs.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TH2.h"

#include <map>
#include <memory>
#include <numeric>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
void DebugSide()
{
    ActRoot::DataManager datman {"../../configs/data.conf"};
    auto chain {datman.GetChain(ActRoot::ModeType::ECorrect)};
    auto chain2 {datman.GetChain(ActRoot::ModeType::EReadSilMod)};
    chain->AddFriend(chain2.get());
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};

    // Read sil specs
    ActPhysics::SilSpecs specs;
    specs.ReadFile("../../configs/detailedSilicons.conf");
    auto sm {specs.GetLayer("l0").GetSilMatrix()->Clone()};

    auto gated {df.Filter([](ActRoot::ModularData& mod) { return mod.Get("GATCONF") == 8; }, {"ModularData"})
                    .Filter("fLightIdx != -1")};
    auto hSP {gated.Histo2D(HistConfig::SP, "fSP.fCoordinates.fX", "fSP.fCoordinates.fZ")};
    // Get means of histograms
    auto model {HistConfig::SP.GetHistogram()};
    std::map<int, ROOT::TThreadedObject<TH2D>> hs;
    for(auto s : {3, 4, 5})
        hs.emplace(s, *model);
    gated.Foreach(
        [&](ActRoot::MergerData& d)
        {
            auto idx {d.fSilNs.front()};
            if(hs.count(idx))
                hs[d.fSilNs.front()].Get()->Fill(d.fSP.X(), d.fSP.Z());
        },
        {"MergerData"});
    // Get means
    std::vector<double> means;
    for(auto& [i, h] : hs)
    {
        auto m {h.Merge()};
        means.push_back(m->GetMean(2));
    }
    auto mean {std::accumulate(means.begin(), means.end(), 0.) / means.size()};
    sm->MoveZTo(mean, {3, 4, 5});

    auto* c0 {new TCanvas {"c0", "DebugFront canvas"}};
    hSP->DrawClone("colz");
    sm->Draw();

    // Save
    hSP->SaveAs("./Outputs/side_sps.root");
    c0->SaveAs("./Pictures/side_sps.png");
}
