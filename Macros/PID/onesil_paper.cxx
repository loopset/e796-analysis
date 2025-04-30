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
void onesil_paper()
{
    // ROOT::EnableImplicitMT();
    // Read data
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EReadSilMod};
    datman.SetRuns(155, 175);
    auto chain {datman.GetChain()};
    auto chain2 {datman.GetChain(ActRoot::ModeType::EMerge)};
    chain->AddFriend(chain2.get());
    ROOT::RDataFrame df {*chain};

    // SilSpecs for thresholds
    auto specs {std::make_shared<ActPhysics::SilSpecs>()};
    specs->ReadFile("../../configs/detailedSilicons.conf");
    // And now apply cuts
    auto twosils {df.Filter(
                        [&](ActRoot::SilData& data, ActRoot::ModularData& mod)
                        {
                            if(mod.Get("GATCONF") != 4)
                                return false;
                            data.ApplyFinerThresholds(specs);
                            auto& ns {data.fSiN};
                            if(ns.size() == 1 && ns.count("f0"))
                            {
                                if(ns["f0"].size() == 1)
                                    if(ns["f0"].front() == 5)
                                        return true;
                            }
                            return false;
                        },
                        {"SilData", "ModularData"})
                      .Define("E0", [](ActRoot::SilData& data) { return data.fSiE["f0"].front(); }, {"SilData"})
                      .Define("N0", [](ActRoot::SilData& data) { return data.fSiN["f0"].front(); }, {"SilData"})};

    // Book histograms
    auto hPID {twosils.Histo1D("E0")};
    // Cuts
    std::pair<double, double> esil {6, 8};
    auto silGated {twosils.Filter(
        [&](float e0)
        {
            if(esil.first <= e0 && e0 <= esil.second)
                return true;
            return false;
        },
        {"E0"})};
    // Write entries
    std::ofstream streamer {"./onesil.dat"};
    silGated.Foreach([&](float e0, ActRoot::MergerData& d) { d.Stream(streamer); }, {"E0", "MergerData"});
    // Save to root file
    silGated.Snapshot("SimpleTree", "./Outputs/onesil.root", {"fRun", "fEntry", "E0", "N0"});

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
