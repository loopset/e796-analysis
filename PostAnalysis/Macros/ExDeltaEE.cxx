#include "ActKinematics.h"
#include "ActLine.h"
#include "ActMergerData.h"
#include "ActParticle.h"
#include "ActSRIM.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TString.h"

#include <string>

#include "../../Selector/Selector.h"
#include "../Gates.cxx"
#include "../HistConfig.h"

void ExDeltaEE()
{
    std::string beam {"20O"};
    std::string target {"2H"};
    std::string light {"3H"};
    ROOT::EnableImplicitMT();
    // Read data
    auto inname {TString::Format("./Outputs/tree_twosils_%s_%s.root", target.c_str(), light.c_str())};
    ROOT::RDataFrame df {"PID_Tree", inname.Data()};

    auto* srim {new ActPhysics::SRIM};
    srim->ReadTable(
        light,
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", light.c_str()).Data());
    srim->ReadTable(
        beam,
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", beam.c_str()).Data());

    // Build energy at vertex
    auto def {df.Define("EVertex",
                        [&](const ActRoot::MergerData& d, float e1, float e0)
                        {
                            // 1-> Sil1 to 0
                            ActRoot::Line line {d.fRP, d.fSP};
                            auto silPoint1 {line.MoveToX(391)}; // pos of f1 from (0,0)
                            auto dist10 {(silPoint1 - d.fSP).R()};
                            auto EAfter0 {srim->EvalInitialEnergy(light, e1, dist10)};
                            auto EVertex {srim->EvalInitialEnergy(light, (e0 + EAfter0), d.fTrackLength)};
                            return EVertex;
                        },
                        {"MergerData", "E1", "E0"})};

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
                             return vkins[slot].ReconstructExcitationEnergy(EVertex, d.fThetaLight * TMath::DegToRad());
                         },
                         {"MergerData", "EVertex", "EBeam"});
    def = def =
        def.DefineSlot("ThetaCM",
                       [&](unsigned int slot, const ActRoot::MergerData& d, double EVertex, double EBeam)
                       {
                           vkins[slot].SetBeamEnergy(EBeam);
                           return vkins[slot].ReconstructTheta3CMFromLab(EVertex, d.fThetaLight * TMath::DegToRad()) *
                                  TMath::RadToDeg();
                       },
                       {"MergerData", "EVertex", "EBeam"});

    // Apply selector
    auto gated {def.Filter([&](ActRoot::MergerData& d) { return E796Gates::rp(d.fRP.X()); }, {"MergerData"})};

    // Book new histograms
    auto hKin {gated.Histo2D(HistConfig::Kin, "fThetaLight", "EVertex")};

    auto hKinCM {gated.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};

    auto hEx {gated.Histo1D(HistConfig::Ex, "Ex")};

    auto hRP {gated.Histo2D(HistConfig::RP, "fRP.fCoordinates.fX", "fRP.fCoordinates.fY")};

    auto hExRP {gated.Histo2D(HistConfig::ExRPx, "fRP.fCoordinates.fX", "Ex")};

    // Save
    auto outname {TString::Format("./Outputs/tree_ex_twosils_%s_%s.root", target.c_str(), light.c_str())};
    gated.Snapshot("Sel_Tree", outname);

    auto* c22 {new TCanvas("c22", "Pipe2 canvas 2")};
    c22->DivideSquare(6);
    c22->cd(1);
    hKin->DrawClone("colz");
    c22->cd(2);
    hKinCM->DrawClone("colz");
    c22->cd(3);
    hRP->DrawClone("colz");
    c22->cd(4);
    hExRP->DrawClone("colz");
    c22->cd(5);
    hEx->DrawClone();
}
