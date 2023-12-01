#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include "Math/Point3Dfwd.h"

#include <fstream>
void GetCorrectedPID()
{
    // ROOT::EnableImplicitMT();
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

    // Read veto cuts
    ActRoot::CutsManager<int> cut;
    cut.ReadCut(0, "./Cuts/debug_banana.root");

    // Apply them
    auto debug {f0.Filter([&](const ROOT::RVecF& silE, float q) { return cut.IsInside(0, silE.front(), q); },
                          {"fSilEs", "fQave"})};

    // Book histograms
    auto hPID {f0.Histo2D({"hPID", "PID for front layer;E_{Si0} [MeV];Q_{ave} [mm^{-1}]", 500, 0, 40, 800, 0, 2000},
                          "ESil0", "fQave")};
    auto hSP {f0.Histo2D({"hSP", "ESil1 == 0 SP;Y [mm];Z [mm]", 200, -20, 300, 200, -20, 300}, "fSP.fCoordinates.fY",
                         "fSP.fCoordinates.fZ")};

    // With cut
    auto hDebug {debug.Histo2D({"hDebug", "SPs from cut;Y [mm];Z [mm]", 200, -20, 300, 200, -20, 300},
                               "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Write to streamer
    // std::ofstream streamer {"./debug_hes.dat"};
    // debug.Foreach(
    //     [&](const ActRoot::MergerData& d)
    //     {
    //         if(cut.IsInside(0, d.fSilEs.front(), d.fQave))
    //             streamer << d.fRun << " " << d.fEntry << '\n';
    //     },
    //     {"MergerData"});
    // streamer.close();

    // plot
    auto* c1 {new TCanvas("c1", "PID canvas")};
    c1->DivideSquare(4);
    c1->cd(1);
    hPID->DrawClone("colz");
    cut.DrawAll();
    c1->cd(2);
    hSP->DrawClone("colz");
    // vetos.DrawAll();
    c1->cd(3);
    hDebug->DrawClone("colz");
    c1->cd(4);
    hSP->DrawClone("colz");
    hDebug->SetMarkerColor(kRed);
    hDebug->SetMarkerStyle(6);
    hDebug->DrawClone("scat same");
}
