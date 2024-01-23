#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"
#include "ActSilMatrix.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include "Math/Point3Dfwd.h"

#include <fstream>

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

    // Read veto cuts
    ActPhysics::SilMatrix sm;
    sm.Read("./antiveto_matrix.root");

    auto vetoed {f0.Filter([&](const ActRoot::MergerData& d)
                           { return sm.IsInside(d.fSilNs.front(), d.fSP.Y(), d.fSP.Z()); },
                           {"MergerData"})};

    // Book histograms
    auto hPID {vetoed.Histo2D({"hPID", "PID for front layer;E_{Si0} [MeV];Q_{ave} [mm^{-1}]", 500, 0, 40, 800, 0, 2000},
                          "ESil0", "fQave")};
    auto hSP {vetoed.Histo2D({"hSP", "ESil1 == 0 SP;Y [mm];Z [mm]", 200, -20, 300, 200, -20, 300}, "fSP.fCoordinates.fY",
                         "fSP.fCoordinates.fZ")};

    // plot
    auto* c1 {new TCanvas("c1", "PID canvas")};
    c1->DivideSquare(4);
    c1->cd(1);
    hPID->DrawClone("colz");
    c1->cd(2);
    hSP->DrawClone("colz");
    sm.SetSyle();
    sm.Draw("same");
}
