#include "ActJoinData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TH2.h"
#include "TROOT.h"

void PlotSP()
{
    ROOT::EnableImplicitMT();

    ActRoot::JoinData data {"../configs/merger.runs"};
    ROOT::RDataFrame d {*data.Get()};
    // d.Describe().Print();

    // Gate on events stopped in first layer
    auto df {d.Filter("fSilLayers.size() == 1")};

    // Book histograms
    auto hF0 {
        df.Filter("fSilLayers.front() == \"f0\"")
            .Histo2D({"hF0", "F0 SPs", 200, -20, 300, 200, -20, 300}, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};
    auto hL0 {
        df.Filter("fSilLayers.front() == \"l0\"")
            .Histo2D({"hL0", "L0 SPs", 200, -20, 276, 200, -20, 276}, "fSP.fCoordinates.fX", "fSP.fCoordinates.fZ")};
    // FOr computing ZOffset
    auto hZOff {
        df.Filter("fSilLayers.front() == \"f0\"")
            .Histo1D({"hZOff", "ZOffset", hF0->GetNbinsY(), hF0->GetYaxis()->GetXmin(), hF0->GetYaxis()->GetXmax()},
                     "fSP.fCoordinates.fZ")};

    // plotting
    auto* c1 {new TCanvas("c1", "Silicon points canvas")};
    c1->DivideSquare(4);
    c1->cd(1);
    hF0->DrawClone("colz");
    c1->cd(2);
    hL0->DrawClone("colz");
    c1->cd(3);
    hZOff->DrawClone();
}
