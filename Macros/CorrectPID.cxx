#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <fstream>
#include <string>
void CorrectPID()
{
    ActRoot::JoinData data {"../configs/merger.runs"};
    // ROOT::EnableImplicitMT();
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

    // Read veto cuts
    ActRoot::CutsManager<int> vetos;
    for(int i = 0; i < 11; i++)
        vetos.ReadCut(i, TString::Format("../Cuts/veto_sil%d.root", i));

    // Apply Z offset
    f0 = f0.Define("corrZ", "fSP.fCoordinates.fZ - (float)76.");

    // Apply cuts
    auto vetof0 {f0.Filter([&](const ROOT::RVecF& silN, float y, float z)
                           { return vetos.IsInside(silN.front(), y, z); },
                           {"fSilNs", "fSP.fCoordinates.fY", "corrZ"})};

    // Book histograms
    auto hPID {vetof0.Define("x", "fSilEs.front()")
                   .Histo2D({"hPID", "PID for front layer;E_{Si0} [MeV];Q_{ave} [mm^{-1}]", 300, 0, 40, 500, 0, 2000},
                            "x", "fQave")};
    auto hSP {vetof0.Histo2D({"hSP", "Vetoed SP;Y [mm];Z [mm]", 150, -10, 270, 150, -10, 270}, "fSP.fCoordinates.fY",
                             "corrZ")};
    // store a few in dat file
    ActRoot::CutsManager<int> cut;
    cut.ReadCut(0, "./debug_hes.root");
    std::ofstream streamer {"./debug_hes.dat"};
    vetof0.Foreach(
        [&](const ActRoot::MergerData& d)
        {
            if(cut.IsInside(0, d.fSilEs.front(), d.fQave))
                streamer << d.fRun << " " << d.fEntry << '\n';
        },
        {"MergerData"});
    streamer.close();

    // plot
    auto* c1 {new TCanvas("c1", "PID canvas")};
    c1->DivideSquare(2);
    c1->cd(1);
    hPID->DrawClone("colz");
    cut.DrawAll();
    c1->cd(2);
    hSP->DrawClone("colz");
    vetos.DrawAll();
}
