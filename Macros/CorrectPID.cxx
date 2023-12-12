#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"
#include "ActPIDCorrector.h"
#include "ActSilMatrix.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TAttLine.h"
#include "TCanvas.h"
#include "TH2.h"
#include "TROOT.h"

#include <fstream>
#include <string>
#include <utility>

void CorrectPID()
{
    ActRoot::JoinData data {"../configs/merger.runs"};
    ROOT::EnableImplicitMT();
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

    // Read cuts
    ActPhysics::SilMatrix sm;
    sm.Read("./silmatrix.root");

    // Apply cuts
    auto vetof0 {f0.Filter([&](const ROOT::RVecF& silN, float y, float z)
                           { return sm.IsInside(silN.front(), y, z); },
                           {"fSilNs", "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ"})};

    // Book histograms
    auto hPID {vetof0.Define("x", "fSilEs.front()")
                   .Histo2D({"hPID", "PID for front layer;E_{Si0} [MeV];Q_{ave} [mm^{-1}]", 500, 0, 40, 800, 0, 2000},
                            "x", "fQave")};
    auto hSP {vetof0.Histo2D({"hSP", "Vetoed SP;Y [mm];Z [mm]", 150, -10, 300, 150, -10, 300}, "fSP.fCoordinates.fY",
                             "fSP.fCoordinates.fZ")};

    // Read preliminary PID cuts
    ActRoot::CutsManager<std::string> cuts;
    // // cuts.ReadCut("p", "./Cuts/pid_protons_f0.root");
    // // cuts.ReadCut("d", "./Cuts/pid_deuterons_f0.root");
    cuts.ReadCut("t", "./Cuts/unpid_tritons_f0.root");
    // // cuts.ReadCut("4He", "./Cuts/pid_4He_f0.root");
    //
    // Init PIDCorrector
    auto* hModel {new TH2D("hModel", "PID Corr", 100, -10, 300, 400, 0, 2000)};
    std::pair<double, double> silELimits {3.5, 4.};
    ActPhysics::PIDCorrector pc {"front", cuts.GetListOfKeys(), hModel};
    // Fill it
    vetof0.Foreach(
        [&](const ActRoot::MergerData& data)
        {
            auto silE {data.fSilEs.front()};
            auto z {data.fSP.Z()};
            auto q {data.fQave};
            if(auto key {cuts.GetKeyIsInside(silE, q)}; key.has_value())
                pc.FillHisto(key.value(), z, q, silE, silELimits.first, silELimits.second);
        },
        {"MergerData"});
    // Execute functions
    pc.GetProfiles();
    pc.FitProfiles(75, 275);
    auto pidcorr {pc.GetCorrection()};
    // Save it
    pidcorr.Write("../Calibrations/Actar/pid_corr_tritons_f0.root");

    // Apply correction
    vetof0 = vetof0.Define("corrQave",
                           [&](const ActRoot::MergerData& data) { return pidcorr.Apply(data.fQave, data.fSP.Z()); },
                           {"MergerData"});

    // Book corrected histogram
    auto hPIDCorr {vetof0.Define("x", "fSilEs.front()")
                       .Histo2D({"hPIDCorr", "Corrected PID;E_{Si0} [MeV];Q_{ave} [mm]", 500, 0, 40, 800, 0, 2000}, "x",
                                "corrQave")};

    // plot
    auto* c1 {new TCanvas("c1", "PID canvas")};
    c1->DivideSquare(4);
    c1->cd(1);
    hPID->DrawClone("colz");
    cuts.DrawAll();
    c1->cd(2);
    hSP->DrawClone("colz");
    sm.SetSyle(true, kSolid, 2, 0);
    sm.Draw(true);
    c1->cd(3);
    hPIDCorr->DrawClone("colz");
    // c1->cd(4);
    // hSP->DrawClone("colz");
    // vetos.DrawAll();
    //
    // PIDCorrector canvas
    pc.Draw();
}
