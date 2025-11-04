#include "ActDataManager.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"


#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"
void PlotAntiVetoThesis()
{
    auto filter = [](ActRoot::MergerData& merger)
    {
        auto& siL {merger.fSilLayers};
        auto& siN {merger.fSilNs};
        // Contains energy in F0
        bool isInF0 {std::find(siL.begin(), siL.end(), "f0") != siL.end()};
        // Contains energy in F1
        bool isInF1 {std::find(siL.begin(), siL.end(), "f1") != siL.end()};
        // Force coincidence of index in 0 and 1
        bool shareIndex {};
        if(isInF0 && isInF1)
        {
            shareIndex = (siN[0] == siN[1]);
        }
        return isInF0 && isInF1 && shareIndex;
    };

    // Read data
    ROOT::EnableImplicitMT();
    ActRoot::DataManager datman {"../../configs/data.conf"};
    // datman.SetRuns(155, 175);
    auto chain {datman.GetChain()};
    ROOT::RDataFrame df {*chain};

    // ActRoot::CutsManager<int> cuts;
    // cuts.ReadCut(0, "./debug_sp.root");

    // Filter
    auto gated {df.Filter(filter, {"MergerData"}).Define("SilN", "fSilNs.front()")};

    // Book histograms
    auto hSP {gated.Histo2D(HistConfig::SP, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Silicon matrix
    auto* sm {E796Utils::GetAntiVetoMatrix()};

    // Save to disk
    gated.Snapshot("SP_Tree", "../../Publications/analysis/Inputs/sp_antiveto.root", {"fSP", "SilN"});

    // std::ofstream streamer {"debug_sp.dat"};
    // gated.Foreach(
    //     [&](ActRoot::MergerData& mer)
    //     {
    //         auto& sp {mer.fSP};
    //         auto n {mer.fSilNs.front()};
    //         if(n == 4 && cuts.IsInside(0, sp.Y(), sp.Z()))
    //             mer.Stream(streamer);
    //     },
    //     {"MergerData"});
    // streamer.close();

    // Draw
    auto* c0 {new TCanvas {"c0", "thesis canvas"}};
    hSP->DrawClone("colz");
    sm->Draw();
    // cuts.DrawAll();
}
