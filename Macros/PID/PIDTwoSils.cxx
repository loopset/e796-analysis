#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActSilData.h"
#include "ActSilSpecs.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TVirtualPad.h"

#include <fstream>
#include <memory>
#include <string>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
void PIDTwoSils()
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
    specs->Print();
    // And now apply cuts
    auto twosils {df.Filter(
                        [&](ActRoot::SilData& data, ActRoot::ModularData& mod)
                        {
                            if(mod.Get("GATCONF") != 4)
                                return false;
                            data.ApplyFinerThresholds(specs);
                            auto& ns {data.fSiN};
                            if(ns.size() == 2)
                            {
                                if(ns["f0"].size() == 1 && ns["f1"].size() == 1)
                                    return true;
                            }
                            return false;
                        },
                        {"SilData", "ModularData"})
                      .Define("E0", [](ActRoot::SilData& data) { return data.fSiE["f0"].front(); }, {"SilData"})
                      .Define("E1", [](ActRoot::SilData& data) { return data.fSiE["f1"].front(); }, {"SilData"})};

    // Book histograms
    auto hPID {twosils.Histo2D(HistConfig::PIDTwo, "E1", "E0")};
    // Cuts
    ActRoot::CutsManager<int> cuts;
    cuts.ReadCut(0, "./Cuts/pid_2H_twosils.root");

    // Write entries
    std::ofstream streamer {"./twosils.dat"};
    twosils.Foreach(
        [&](float e0, float e1, ActRoot::MergerData& d)
        {
            if(cuts.IsInside(0, e1, e0))
                d.Stream(streamer);
        },
        {"E0", "E1", "MergerData"});

    // twosils.Snapshot("Simple_Tree", "./twopid.root", {"E0", "E1"});

    // plot
    auto* c1 {new TCanvas("c1", "Two sils PID")};
    gPad->SetLogz();
    hPID->DrawClone("colz");
    cuts.DrawAll();
}
