#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "TCanvas.h"
#include "TChain.h"
#include "TROOT.h"
#include "TString.h"
#include <fstream>
void PlotRP()
{
    // ROOT::EnableImplicitMT();

    auto* chain {new TChain("ACTAR_Merged")};
    for(const auto& run : {155, 156, 157, 158, 159})
        chain->Add(TString::Format("../RootFiles/Merger/Merger_Run_%04d.root", run));
    ROOT::RDataFrame d {*chain};

    // Gate on events stopped in first layer
    auto df {d.Filter("fSilLayers.size() > 0")};

    // Book histograms
    auto hX {df.Histo1D({"hRP_X", "RP.X();X [mm];Counts / 2 mm", 140, -10, 270}, 
                        "fRP.fCoordinates.fX")};
    
    // write to file according to Juan
    std::ofstream streamer {"debug_rp.dat"};
    df.Foreach([&](const ActRoot::MergerData& md){streamer<<md.fRun<<" "<<md.fEntry<<'\n';}, {"MergerData"});
    streamer.close();
    
    // plotting
    auto* c1 {new TCanvas("c1", "RP plots")};
    hX->DrawClone();
    // Write it
    hX->SaveAs("debug_rp.root");
}
