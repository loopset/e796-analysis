#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TChain.h"
#include "TROOT.h"
#include "TString.h"

#include <fstream>
#include <iostream>
void PlotRP()
{
    ActRoot::JoinData data {"../configs/merger.runs"};
    ROOT::RDataFrame d {*data.Get()};

    auto df {d.Filter("fSilLayers.size() == 1")};
    df = df.Filter("fSilEs.front() > 15.");

    // Book histograms
    auto hXY {df.Histo2D({"hXY", "RP on XY;X [mm]; Y [mm]", 200, -5, 270, 200, -5, 270}, "fRP.fCoordinates.fX",
                         "fRP.fCoordinates.fY")};
    auto hXZ {df.Histo2D({"hXZ", "RP on XZ;X [mm];Z [mm]", 200, -5, 270, 200, -5, 270}, "fRP.fCoordinates.fX",
                         "fRP.fCoordinates.fZ")};
    auto hYZ {df.Histo2D({"hYZ", "RP on YZ;y [mm];Z [mm]", 200, -5, 270, 200, -5, 270}, "fRP.fCoordinates.fY",
                         "fRP.fCoordinates.fZ")};

    // Trigger event loop
    std::cout << "Number of events : " << df.Count().GetValue() << '\n';

    // plot
    auto* c1 {new TCanvas("c1", "RP canvas")};
    c1->DivideSquare(3);
    c1->cd(1);
    hXY->DrawClone("colz");
    c1->cd(2);
    hXZ->DrawClone("colz");
    c1->cd(3);
    hYZ->DrawClone("colz");
}
