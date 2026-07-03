#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"

#include <fstream>

#include "../PostAnalysis/HistConfig.h"
#include "../PostAnalysis/Utils.cxx"

void debug_antiveto()
{
    // ROOT::EnableImplicitMT();

    ActRoot::DataManager dataman {"../configs/data.conf"};
    dataman.SetRuns(155, 200);
    auto chain {dataman.GetChain()};

    ROOT::RDataFrame df {*chain};
    auto gated {df.Filter(
        [](ActRoot::MergerData& merger)
        {
            auto& siL {merger.fSilLayers};
            auto& siN {merger.fSilNs};
            // Contains energy in F0
            bool isInF0 {std::find(siL.begin(), siL.end(), "f0") != siL.end()};
            // Contains energy in F1
            bool isInF1 {std::find(siL.begin(), siL.end(), "f1") != siL.end()};
            // Force coincidence of index in 0 and 1
            bool shareIndex {};
            bool included {true};
            if(isInF0 && isInF1)
            {
                shareIndex = (siN[0] == siN[1]);
                if(siN[0] == 1 || siN[0] == 6 || siN[0] == 9)
                    included = false;
            }
            return isInF0 && isInF1 && shareIndex && included;
        },
        {"MergerData"})};

    auto h {gated.Histo2D(HistConfig::SP, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    auto* sm {E796Utils::GetAntiVetoMatrix()};

    ActRoot::CutsManager<int> cuts;
    cuts.ReadCut(0, "./debug_antiveto.root");

    auto dfcut {
        gated.Filter([&](ActRoot::MergerData& m) { return cuts.IsInside(0, m.fSP.Y(), m.fSP.Z()); }, {"MergerData"})};
    std::ofstream streamer {"./debug_antiveto.dat"};
    dfcut.Foreach([&](ActRoot::MergerData& m) { m.Stream(streamer); }, {"MergerData"});

    auto* c0 {new TCanvas {"c0", "debug antiveto"}};
    h->DrawClone("colz");
    sm->Draw();
    cuts.DrawAll();
}
