#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActSilData.h"
#include "ActSilSpecs.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TLine.h"
#include "TROOT.h"
#include "TVirtualPad.h"

#include <fstream>
#include <memory>
#include <string>
#include <utility>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
void side_paper()
{
    // ROOT::EnableImplicitMT();
    // Read data
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EReadSilMod};
    datman.SetRuns(155, 165);
    auto chain {datman.GetChain()};
    auto chain2 {datman.GetChain(ActRoot::ModeType::EMerge)};
    chain->AddFriend(chain2.get());
    ROOT::RDataFrame df {*chain};

    // SilSpecs for thresholds
    auto specs {std::make_shared<ActPhysics::SilSpecs>()};
    specs->ReadFile("../../configs/detailedSilicons.conf");
    // And now apply cuts
    auto gated {df.Filter(
                      [&](ActRoot::SilData& data, ActRoot::ModularData& mod)
                      {
                          if(mod.Get("GATCONF") != 8)
                              return false;
                          data.ApplyFinerThresholds(specs);
                          auto& ns {data.fSiN};
                          if(ns.size() == 1 && ns.count("l0"))
                          {
                              if(ns["l0"].size() == 1)
                                  return true;
                          }
                          return false;
                      },
                      {"SilData", "ModularData"})
                    .Define("E0", [](ActRoot::SilData& data) { return data.fSiE["l0"].front(); }, {"SilData"})
                    .Define("N0", [](ActRoot::SilData& data) { return data.fSiN["l0"].front(); }, {"SilData"})};

    // Book histograms
    auto hPID {gated.Histo1D("E0")};
    // Cuts
    std::pair<double, double> esil {0, 100};
    auto silGated {gated.Filter(
        [&](float e0)
        {
            if(esil.first <= e0 && e0 <= esil.second)
                return true;
            return false;
        },
        {"E0"})};
    // Write entries
    std::ofstream streamer {"./sidesil.dat"};
    silGated.Foreach([&](float e0, ActRoot::MergerData& d) { d.Stream(streamer); }, {"E0", "MergerData"});
    // Save to root file
    silGated.Snapshot("SimpleTree", "./Outputs/sidesil.root", {"fRun", "fEntry", "E0", "N0"});

    // plot
    auto* c1 {new TCanvas("c1", "Two sils PID")};
    gPad->SetLogz();
    hPID->DrawClone("colz");
    gPad->Update();
    for(const auto& e : {esil.first, esil.second})
    {
        auto* line {new TLine {e, gPad->GetUymin(), e, gPad->GetUymax()}};
        line->SetLineWidth(2);
        line->SetLineColor(8);
        line->Draw();
    }
}
