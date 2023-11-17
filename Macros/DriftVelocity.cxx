#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TROOT.h"

#include <fstream>
#include <iostream>
#include <string>

void DriftVelocity()
{
    // ROOT::EnableImplicitMT();

    // Get data
    ActRoot::JoinData join {"./../configs/merger.runs"};
    // join->Print();
    ROOT::RDataFrame d {*join.Get()};
    // ROOT::RDataFrame d {"ACTAR_Merged", "./../RootFiles/Merger/Merger_Run_0156.root"};
    // d.Describe().Print();
    // gate on events stopped in first sil layer
    auto df {
        d.Filter([](const ROOT::VecOps::RVec<std::string>& layers) { return layers.size() == 1 ; }, {"fSilLayers"})};

    // Config histograms
    auto hLeft {df.Filter("fSilLayers.front() == \"l0\"")
                    .Histo2D({"hLeft", "Left;X [pad];Z [tb]", 200, -20, 150, 200, -20, 150}, "fSP.fCoordinates.fX",
                             "fSP.fCoordinates.fZ")};

    auto hFront {df.Filter("fSilLayers.front() == \"f0\"")
                     .Histo2D({"hFront", "Front;Y [pad];Z [tb]", 200, -20, 150, 200, -20, 150}, "fSP.fCoordinates.fY",
                              "fSP.fCoordinates.fZ")};

    auto hFront1 {df.Filter("fSilLayers.front() == \"f1\"")
                      .Histo2D({"hFront1", "Front 1;Y [pad];Z [tb]", 200, -20, 150, 200, -20, 150}, "fSP.fCoordinates.fY",
                               "fSP.fCoordinates.fZ")};

    auto hRun {df.Histo1D("fRun")};

    auto hEntry {df.Histo1D("fEntry")};

    std::cout << "hFront1 entries : " << hFront1->GetEntries() << '\n';

    // Write entries
    ActRoot::CutsManager<int> cuts;
    cuts.ReadCut(0, "./central_clusters.root");
    std::ofstream streamer {"./central_clusters.dat"};
    df.Foreach(
        [&](const ActRoot::MergerData& data)
        {
            if((cuts.IsInside(0, data.fSP.Y(), data.fSP.Z())))
                streamer << data.fRun << " " << data.fEntry << '\n';
        },
        {"MergerData"});
    streamer.close();

    // plotting
    auto* c0 {new TCanvas("c0", "Debug canvas")};
    c0->DivideSquare(2);
    c0->cd(1);
    hRun->DrawClone();
    c0->cd(2);
    hEntry->DrawClone();
    auto* c1 {new TCanvas("c1", "Preliminary SP")};
    c1->DivideSquare(2);
    c1->cd(1);
    hLeft->DrawClone("colz");
    c1->cd(2);
    hFront->DrawClone("colz");
    cuts.SetLineAttributes(0, kRed, 2);
    cuts.DrawAll();
}
