#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"
#include "ActPIDCorrector.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

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
                   .Histo2D({"hPID", "PID for front layer;E_{Si0} [MeV];Q_{ave} [mm^{-1}]", 500, 0, 40, 800, 0, 2000},
                            "x", "fQave")};
    auto hSP {vetof0.Histo2D({"hSP", "Vetoed SP;Y [mm];Z [mm]", 150, -10, 270, 150, -10, 270}, "fSP.fCoordinates.fY",
                             "corrZ")};

    // Read preliminary PID cuts
    ActRoot::CutsManager<std::string> cuts;
    // cuts.ReadCut("p", "./Cuts/pid_protons_f0.root");
    // cuts.ReadCut("d", "./Cuts/pid_deuterons_f0.root");
    cuts.ReadCut("t", "./Cuts/pid_tritons_f0.root");
    // cuts.ReadCut("4He", "./Cuts/pid_4He_f0.root");

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
    hPIDCorr->DrawClone("colz");
    c1->cd(3);
    hSP->DrawClone("colz");
    vetos.DrawAll();

    // PIDCorrector canvas
    pc.Draw();
}
