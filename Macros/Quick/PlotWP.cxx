#include "ActDataManager.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TVirtualPad.h"

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"

void PlotWP()
{
    ROOT::EnableImplicitMT();

    ActRoot::DataManager data {"../../configs/data.conf"};
    auto chain {data.GetJoinedData()};
    ROOT::RDataFrame df {*chain};

    // Filter
    auto beam {df.Filter("fSilLayers.size() > 0")
                   .Filter([](const ActRoot::MergerData& d) { return d.fRP.x() > 26; }, {"MergerData"})};

    auto hWP {beam.Histo2D(HistConfig::ChangeTitle(HistConfig::SP, "Window point"), "fWP.fCoordinates.fY",
                           "fWP.fCoordinates.fZ")};
    auto hZThetaZ {beam.Histo2D(HistConfig::ZThetaZ, "fWP.fCoordinates.fZ", "fThetaBeamZ")};
    auto hYphiY {beam.Histo2D(HistConfig::YPhiY, "fWP.fCoordinates.fY", "fPhiBeamY")};

    // plotting
    auto* c0 {new TCanvas("c0", "WP canvas")};
    c0->DivideSquare(4);
    c0->cd(1);
    gPad->SetLogz();
    hWP->DrawClone("colz");
    c0->cd(2);
    gPad->SetLogz();
    hZThetaZ->DrawClone("colz");
    c0->cd(3);
    gPad->SetLogz();
    hYphiY->DrawClone("colz");
}
