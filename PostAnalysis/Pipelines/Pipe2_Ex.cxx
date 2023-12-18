#include "ActColors.h"
#include "ActCutsManager.h"
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

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

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
    srim->ReadInterpolations(
        beam,
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/transformed/%s_in_952mb_mixture.dat", beam.c_str())
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

    // Init particles
    ActPhysics::Particle pb {beam};
    ActPhysics::Particle pt {target};
    ActPhysics::Particle pl {light};

    // Build beam energy
    def = def.Define("EBeam",
                     [&](const ActRoot::MergerData& d)
                     {
                         double RIni {srim->EvalDirect(beam, 35 * pb.GetAMU())};
                         double RVertex {RIni - d.fRP.X()};
                         return srim->EvalInverse(beam, RVertex);
                     },
                     {"MergerData"});

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
                             return vkins[slot].ReconstructExcitationEnergy(EVertex, d.fThetaLight * TMath::DegToRad());
                         },
                         {"MergerData", "EVertex", "EBeam"});

    // Book new histograms
    auto hKin {def.Histo2D((isSide) ? HistConfig::KinEl : HistConfig::Kin, "fThetaLight", "EVertex")};
    auto hEx {def.Histo1D(HistConfig::Ex, "Ex")};

    // // Write
    // ActRoot::CutsManager<int> cut;
    // cut.ReadCut(0, "/media/Data/E796v2/PostAnalysis/Cuts/debug_d_d.root");
    // std::ofstream streamer {"/media/Data/E796v2/PostAnalysis/debug_d_d.dat"};
    // def.Foreach(
    //     [&](const ActRoot::MergerData& d, double EVertex)
    //     {
    //         if(cut.IsInside(0, d.fThetaLight, EVertex))
    //             streamer << d.fRun << " " << d.fEntry << '\n';
    //     },
    //     {"MergerData", "EVertex"});
    // streamer.close();

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
