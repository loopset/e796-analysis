#ifndef Pipe2_Ex_cxx
#define Pipe2_Ex_cxx

#include "ActColors.h"
#include "ActKinematics.h"
#include "ActMergerData.h"
#include "ActParticle.h"
#include "ActSRIM.h"

#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"
#include "TString.h"

#include <ios>
#include <iostream>
#include <string>
#include <vector>

#include "../HistConfig.h"
// #include "../Utils.cxx"
#include "../../Selector/Selector.h"

void Pipe2_Ex(const std::string& beam, const std::string& target, const std::string& light, bool isSide)
{
    // Use beam-corrected or legacy fThetaLight
    bool debug {false};
    std::cout << BOLDYELLOW << "is DebugTheta enabled ? " << std::boolalpha << debug << '\n';

    // Read data
    auto filename {gSelector->GetAnaFile(1, beam, target, light, false)};
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"PID_Tree", filename};

    // Book histograms
    auto hPID {df.Define("ESil0", "fSilEs.front()").Histo2D(HistConfig::PID, "ESil0", "fQave")};

    // Init SRIM
    auto* srim {new ActPhysics::SRIM};
    srim->ReadTable(
        light,
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", light.c_str()).Data());
    srim->ReadTable(
        beam,
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", beam.c_str()).Data());

    // Build energy at vertex
    auto def = df.Define("EVertex", [&](const ActRoot::MergerData& d)
                         { return srim->EvalInitialEnergy(light, d.fSilEs.front(), d.fTrackLength); }, {"MergerData"});

    // Init particles
    ActPhysics::Particle pb {beam};
    ActPhysics::Particle pt {target};
    ActPhysics::Particle pl {light};

    // Build beam energy
    def = def.Define("EBeam", [&](const ActRoot::MergerData& d)
                     { return srim->Slow(beam, 35 * pb.GetAMU(), d.fRP.X()); }, {"MergerData"});

    ActPhysics::Kinematics kin {pb, pt, pl, 35 * pb.GetAMU()};
    // Vector of kinematics as one object is needed per
    // processing slot (since we are changing EBeam in each entry)
    std::vector<ActPhysics::Kinematics> vkins {def.GetNSlots()};
    for(auto& k : vkins)
        k = kin;
    def = def.DefineSlot("Ex",
                         [&](unsigned int slot, const ActRoot::MergerData& d, double EVertex, double EBeam)
                         {
                             vkins[slot].SetBeamEnergy(EBeam);
                             return vkins[slot].ReconstructExcitationEnergy(
                                 EVertex, ((debug) ? d.fThetaDebug : d.fThetaLight) * TMath::DegToRad());
                         },
                         {"MergerData", "EVertex", "EBeam"});
    def =
        def.DefineSlot("ExLegacy",
                       [&](unsigned int slot, const ActRoot::MergerData& d, double EVertex, double EBeam)
                       {
                           vkins[slot].SetBeamEnergy(EBeam);
                           return vkins[slot].ReconstructExcitationEnergy(EVertex, d.fThetaLegacy * TMath::DegToRad());
                       },
                       {"MergerData", "EVertex", "EBeam"});
    def = def.DefineSlot("ThetaCM",
                         [&](unsigned int slot, const ActRoot::MergerData& d, double EVertex, double EBeam)
                         {
                             vkins[slot].SetBeamEnergy(EBeam);
                             return vkins[slot].ReconstructTheta3CMFromLab(
                                        EVertex, ((debug) ? d.fThetaDebug : d.fThetaLight) * TMath::DegToRad()) *
                                    TMath::RadToDeg();
                         },
                         {"MergerData", "EVertex", "EBeam"});
    def =
        def.DefineSlot("ThetaCMLegacy",
                       [&](unsigned int slot, const ActRoot::MergerData& d, double EVertex, double EBeam)
                       {
                           vkins[slot].SetBeamEnergy(EBeam);
                           return vkins[slot].ReconstructTheta3CMFromLab(EVertex, d.fThetaLegacy * TMath::DegToRad()) *
                                  TMath::RadToDeg();
                       },
                       {"MergerData", "EVertex", "EBeam"});

    // Book new histograms
    auto hKin {def.Histo2D((isSide) ? HistConfig::KinEl : HistConfig::Kin, (debug) ? "fThetaDebug" : "fThetaLight",
                           "EVertex")};

    auto hKinCM {def.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};

    auto hEBeam {def.Histo1D("EBeam")};
    auto hEx {def.Histo1D(HistConfig::Ex, "Ex")};

    auto hTheta {def.Histo1D((debug) ? "fThetaDebug" : "fThetaLight")};

    auto hThetaBeam {def.Histo2D(HistConfig::ThetaBeam, "fRP.fCoordinates.fX", "fThetaBeam")};

    auto hRP {def.Histo2D(HistConfig::RP, "fRP.fCoordinates.fX", "fRP.fCoordinates.fY")};

    auto hThetaCMLab {def.Histo2D(HistConfig::ThetaCMLab, "fThetaLight", "ThetaCM")};

    // Ex dependences
    auto hExThetaCM {def.Histo2D(HistConfig::ExThetaCM, "ThetaCM", "Ex")};
    auto hExThetaLab {def.Histo2D(HistConfig::ExThetaLab, "fThetaLight", "Ex")};
    auto hExThetaLegacy {
        def.Histo2D(HistConfig::ChangeTitle(HistConfig::ExThetaLab, "E_{x}^{Legacy} vs #theta_{Legacy}"),
                    "fThetaLegacy", "ExLegacy")};
    auto hExRP {def.Histo2D(HistConfig::ExRPx, "fRP.fCoordinates.fX", "Ex")};

    // Heavy histograms
    auto hThetaHLLab {def.Histo2D(HistConfig::ChangeTitle(HistConfig::ThetaHeavyLight, "Lab correlations"),
                                  "fThetaLight", "fThetaHeavy")};
    // Save!
    auto outfile {gSelector->GetAnaFile(2, beam, target, light, false)};
    def.Snapshot("Final_Tree", outfile);

    // Save also RP histogram!
    // auto* pRPx {hRP->ProjectionX("px")};
    // pRPx->SaveAs(E796Utils::GetFileName(2, beam, target, light, isSide, "rpx"));

    // plot
    auto* c20 {new TCanvas("c20", "Pipe2 canvas 0")};
    hPID->DrawClone("colz");

    auto* c22 {new TCanvas("c22", "Pipe2 canvas 2")};
    c22->DivideSquare(4);
    c22->cd(1);
    hTheta->DrawClone();
    c22->cd(2);
    hThetaBeam->DrawClone("colz");
    c22->cd(3);
    hRP->DrawClone("colz");
    c22->cd(4);
    hEBeam->DrawClone();

    auto* c21 {new TCanvas("c21", "Pipe2 canvas 1")};
    c21->DivideSquare(6);
    c21->cd(1);
    hKin->DrawClone("colz");
    auto* theo {kin.GetKinematicLine3()};
    theo->Draw("same");
    c21->cd(2);
    hEx->DrawClone();
    c21->cd(3);
    hKinCM->DrawClone("colz");
    c21->cd(4);
    hExThetaLab->DrawClone("colz");
    c21->cd(5);
    hExThetaCM->DrawClone("colz");
    c21->cd(6);
    hExRP->DrawClone("colz");

    auto* c23 {new TCanvas {"c23", "Pipe2 canvas 3"}};
    c23->DivideSquare(4);
    c23->cd(1);
    hThetaHLLab->DrawClone("colz");
    c23->cd(2);
    hThetaCMLab->DrawClone("colz");
    c23->cd(3);
    hExThetaLegacy->DrawClone("colz");
}
#endif
