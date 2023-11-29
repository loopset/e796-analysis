#include "ActJoinData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
void GetCorrectedPID()
{
    ROOT::EnableImplicitMT();
    ActRoot::JoinData data {"../configs/merger.runs", "../configs/corrections.runs"};
    ROOT::RDataFrame d {*data.Get()};

    // Just for front now with ESil f1 = 0
    auto f0 {d.Filter(
        [](const ROOT::VecOps::RVec<std::string>& silL)
        {
            bool onlyOne {silL.size() == 1};
            bool isF0 {};
            if(onlyOne)
                isF0 = (silL.front() == "f0");
            return onlyOne && isF0;
        },
        {"fSilLayers"})};

    // Define energy in X
    f0 = f0.Define("ESil0", "fSilEs.front()");

    // Book histograms
    auto hPID {f0.Histo2D({"hPID", "PID for front layer;E_{Si0} [MeV];Q_{ave} [mm^{-1}]", 500, 0, 40, 800, 0, 2000},
                          "ESil0", "fQave")};
    auto hSP {f0.Histo2D({"hSP", "Vetoed SP;Y [mm];Z [mm]", 150, -10, 270, 150, -10, 270}, "fSP.fCoordinates.fY",
                         "fSP.fCoordinates.fZ")};

    // plot
    auto* c1 {new TCanvas("c1", "PID canvas")};
    c1->DivideSquare(2);
    c1->cd(1);
    hPID->DrawClone("colz");
    c1->cd(2);
    hSP->DrawClone("colz");
}
