#include "ActColors.h"
#include "ActKinematics.h"
#include "ActMergerData.h"
#include "ActParticle.h"
#include "ActSRIM.h"

#include "ROOT/RDFHelpers.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"
#include "TString.h"

#include <iostream>
#include <string>

#include "../HistConfig.h"
#include "../Utils.cxx"

void Pipe2_Ex(const std::string& beam, const std::string& target, const std::string& light, bool isSide)
{
    auto filename {E796Utils::GetFileName(1, beam, target, light, isSide)};
    std::cout << BOLDGREEN << "Reading file: " << filename << RESET << '\n';
    // Read
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"PID_Tree", filename};

    // Book histograms
    auto hPID {df.Define("ESil0", "fSilEs.front()").Histo2D(HistConfig::PID, "ESil0", "fQave")};

    // Init SRIM
    auto* srim {new ActPhysics::SRIM};
    srim->ReadInterpolations(
        light,
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/transformed/%s_in_952mb_mixture.dat", light.c_str())
            .Data());

    // Build energy at vertex
    auto def = df.Define("EVertex",
                         [&](const ActRoot::MergerData& d)
                         {
                             double RIni {srim->EvalDirect(light, d.fSilEs.front())};
                             double RVertex {RIni + d.fTrackLength};
                             return srim->EvalInverse(light, RVertex);
                         },
                         {"MergerData"});
    // Init Kinematics
    ActPhysics::Particle pb {beam};
    ActPhysics::Kinematics kin {beam, target, light, 35. * pb.GetAMU(), 0};
    kin.Print();
    def = def.Define("Ex",
                     [&](const ActRoot::MergerData& d, double EVertex)
                     { return kin.ReconstructExcitationEnergy(EVertex, d.fThetaLight * TMath::DegToRad()); },
                     {"MergerData", "EVertex"});

    // Book new histograms
    auto hKin {def.Histo2D(HistConfig::Kin, "fThetaLight", "EVertex")};
    auto hEx {def.Histo1D(HistConfig::Ex, "Ex")};

    // Save!
    auto outfile {E796Utils::GetFileName(2, beam, target, light, isSide)};
    def.Snapshot("Final_Tree", outfile);

    // plot
    auto* c20 {new TCanvas("c20", "Pipe2 canvas 0")};
    hPID->DrawClone("colz");

    auto* c21 {new TCanvas("c21", "pipe2 canvas 1")};
    c21->DivideSquare(2);
    c21->cd(1);
    hKin->DrawClone("colz");
    auto* theo {kin.GetKinematicLine3()};
    theo->Draw("same");
    c21->cd(2);
    hEx->DrawClone();
}
