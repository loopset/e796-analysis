#include "ActJoinData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <string>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
void PIDTwoSils()
{
    // ROOT::EnableImplicitMT();
    // Read data
    ActRoot::JoinData data {"./../../configs/merger.runs"};
    ROOT::RDataFrame d {*data.Get()};

    // Filter
    auto twosils {
        d.Filter([](const ROOT::VecOps::RVec<std::string>& silL) { return silL.size() == 2; }, {"fSilLayers"})};

    // Get PID
    auto hPID {twosils.Define("ESil0", "fSilEs[0]")
                   .Define("ESil1", "fSilEs[1]")
                   .Histo2D(HistConfig::PIDTwo, "ESil1", "ESil0")};

    // Get cuts
    // ActRoot::CutsManager<int> cut;
    // cut.ReadCut(0, "./debug_hes.root");
    // std::ofstream streamer {"./debug_hes_twosils.dat"};
    // twosils.Foreach(
    //     [&](const ActRoot::MergerData& d)
    //     {
    //         if(cut.IsInside(0, d.fSilEs[1], d.fSilEs[0]))
    //             streamer << d.fRun << " " << d.fEntry << '\n';
    //     },
    //     {"MergerData"});
    // streamer.close();

    // plot
    auto* c1 {new TCanvas("c1", "Two sils PID")};
    hPID->DrawClone("colz");
}
