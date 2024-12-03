#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActPIDCorrector.h"
#include "ActSilMatrix.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TH2.h"
#include "TROOT.h"
#include "TString.h"

#include <string>
#include <utility>

#include "/media/Data/E796v2/PostAnalysis/Gates.cxx"
#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"

void CorrectPID(bool write)
{
    ROOT::EnableImplicitMT();
    ActRoot::DataManager data {"../../configs/data.conf", ActRoot::ModeType::EMerge};
    auto chain {data.GetJoinedData()};
    ROOT::RDataFrame d {*chain};

    // Just for front now with ESil f1 = 0
    auto f0 {d.Filter(E796Gates::front0, {"MergerData"})};

    // Which particle to gate on?
    std::string target {"2H"};
    std::string light {"3H"};
    // Read silicon matrix
    auto* sm {E796Utils::GetEffSilMatrix(target, light)};

    // Apply cuts
    auto vetof0 {f0.Filter([&](const ROOT::RVecF& silN, float y, float z) { return sm->IsInside(silN.front(), y, z); },
                           {"fSilNs", "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ"})};

    // Book histograms
    auto hPID {vetof0.Define("x", "fSilEs.front()").Histo2D(HistConfig::PID, "x", "fQave")};
    auto hSP {vetof0.Histo2D(HistConfig::SP, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Read preliminary PID cuts
    ActRoot::CutsManager<std::string> cuts;
    cuts.ReadCut(light, "./Cuts/unpid_tritons_f0.root");


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
    if(write)
        pidcorr.Write("/media/Data/E796v2/Calibrations/Actar/pid_corr_tritons_f0.root");

    // Apply correction
    vetof0 = vetof0.Define("corrQave", [&](const ActRoot::MergerData& data)
                           { return pidcorr.Apply(data.fQave, data.fSP.Z()); }, {"MergerData"});

    // Book corrected histogram
    auto hPIDCorr {vetof0.Define("x", "fSilEs.front()")
                       .Histo2D(HistConfig::ChangeTitle(HistConfig::PID, "Corrected PID"), "x", "corrQave")};

    // plot
    auto* c1 {new TCanvas("c1", "PID canvas")};
    c1->DivideSquare(4);
    c1->cd(1);
    hPID->DrawClone("colz");
    cuts.DrawAll();
    c1->cd(2);
    hSP->DrawClone("colz");
    sm->Draw();
    c1->cd(3);
    hPIDCorr->DrawClone("colz");
    // c1->cd(4);
    // hSP->DrawClone("colz");
    // vetos.DrawAll();
    //
    // PIDCorrector canvas
    pc.Draw();
}
