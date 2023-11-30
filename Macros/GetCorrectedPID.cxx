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
    ActRoot::CutsManager<int> vetos;
    for(int i = 0; i < 11; i++)
        vetos.ReadCut(i, TString::Format("../Cuts/veto_sil%d.root", i));

    // Apply them
    auto bv {f0.Filter(
        [&](const ROOT::RVecF& silN, const ROOT::Math::XYZPointF& sp)
        {
            return true; // vetos.IsInside(silN.front(), sp.Y(), sp.Z());
        },
        {"fSilNs", "fSP"})};

    auto vetoed {bv.Filter("fSilNs.front() == 10")};
    // Book histograms
    auto hPID {vetoed.Histo2D({"hPID", "PID for front layer;E_{Si0} [MeV];Q_{ave} [mm^{-1}]", 500, 0, 40, 800, 0, 2000},
                              "ESil0", "fQave")};
    auto hSP {vetoed.Histo2D({"hSP", "Vetoed SP;Y [mm];Z [mm]", 200, -20, 300, 200, -20, 300}, "fSP.fCoordinates.fY",
                             "fSP.fCoordinates.fZ")};

    // Write to streamer
    // std::ofstream streamer {"./debug_banana.dat"};
    // ActRoot::CutsManager<int> cuts;
    // cuts.ReadCut(0, "./Cuts/debug_banana.root");
    // vetoed.Foreach(
    //     [&](const ActRoot::MergerData& d)
    //     {
    //         if(cuts.IsInside(0, d.fSilEs.front(), d.fQave))
    //             streamer << d.fRun << " " << d.fEntry << '\n';
    //     },
    //     {"MergerData"});
    // streamer.close();

    // plot
    auto* c1 {new TCanvas("c1", "PID canvas")};
    c1->DivideSquare(2);
    c1->cd(1);
    hPID->DrawClone("colz");
    // cuts.DrawAll();
    c1->cd(2);
    hSP->DrawClone("colz");
    vetos.DrawAll();
}
