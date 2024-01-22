#include "ActJoinData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"

void PlotWP()
{
    ROOT::EnableImplicitMT();

    ActRoot::JoinData data {"../configs/merger.runs"};

    ROOT::RDataFrame df {*data.Get()};

    // Filter
    auto beam {df.Filter("fSilLayers.size() > 1")};

    auto hWP {df.Histo2D(HistConfig::ChangeTitle(HistConfig::SP, "Window point"), "fWP.fCoordinates.fY",
                         "fWP.fCoordinates.fZ")};
    auto hZThetaZ {df.Histo2D(HistConfig::ZThetaZ, "fWP.fCoordinates.fZ", "fThetaBeamZ")};
    auto hYphiY {df.Histo2D(HistConfig::YPhiY, "fWP.fCoordinates.fY", "fPhiBeamY")};

    // plotting
    auto* c0 {new TCanvas("c0", "WP canvas")};
    c0->DivideSquare(4);
    c0->cd(1);
    hWP->DrawClone("colz");
    c0->cd(2);
    hZThetaZ->DrawClone("colz");
    c0->cd(3);
    hYphiY->DrawClone("colz");
}
