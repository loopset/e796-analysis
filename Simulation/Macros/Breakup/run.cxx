#include "ActConstants.h"
#include "ActDecayGenerator.h"
#include "ActKinematicGenerator.h"
#include "ActKinematics.h"
#include "ActParticle.h"
#include "ActSRIM.h"
#include "ActSilMatrix.h"
#include "ActSilSpecs.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TMath.h"
#include "TROOT.h"
#include "TRandom.h"
#include "TVirtualPad.h"

#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3D.h"
#include "Math/Vector3Dfwd.h"
#include <memory>


#include "../../../PostAnalysis/HistConfig.h"

using XYZVector = ROOT::Math::XYZVector;
using XYZPoint = ROOT::Math::XYZPoint;

XYZPoint SampleVertex()
{
    auto x {gRandom->Uniform() * 256};
    auto y {gRandom->Gaus(128, 2)};
    auto z {gRandom->Gaus(128, 2)};
    return {x, y, z};
}

bool ReachesSilicon(ActPhysics::SilSpecs* specs, ActPhysics::SRIM* srim, const XYZPoint& vertex, double T, double theta,
                    double phi)
{
    // Cut on vertex
    if(!(40 <= vertex.X() && vertex.X() <= 200))
        return false;
    // Geometrical acceptance
    ROOT::Math::XYZVector dirBeamFrame {TMath::Cos(theta), TMath::Sin(theta) * TMath::Sin(phi),
                                        TMath::Sin(theta) * TMath::Cos(phi)};
    auto [silIndex0, silPoint0InMM] {specs->FindSPInLayer("l0", vertex, dirBeamFrame)};
    // skip tracks that doesn't reach silicons or are not in SiliconMatrix indexes
    if(silIndex0 == -1)
        return false;
    // Mask silicon number
    if(silIndex0 == 0 || silIndex0 == 3 || silIndex0 == 6)
        return false;
    // Define SP distance
    auto distance0 {(vertex - silPoint0InMM).R()};
    auto T3EnteringSil {srim->SlowWithStraggling("light", T, distance0)};
    if(T3EnteringSil <= 0)
        return false;
    // ELoss in silicon
    auto angleNormal {TMath::ACos(dirBeamFrame.unit().Dot(XYZVector {0, 1, 0}))};
    auto T3AfterSil0 {srim->SlowWithStraggling("lightInSil", T3EnteringSil,
                                               specs->GetLayer("l0").GetUnit().GetThickness(), angleNormal)};
    auto eLoss0 {T3EnteringSil - T3AfterSil0};
    // Cut in ELoss in silicon
    if(!(0.7 <= eLoss0 && eLoss0 <= 5.8))
        return false;
    // No punchthrough
    if(T3AfterSil0 > 0)
        return false;

    return true;
}

void run()
{
    // Beam energy
    double EBeam {700}; // MeV

    // Init generators
    // Phase space: 20O + d -> 20O + p + n
    auto* phase {new ActSim::KinematicGenerator {"20O", "2H", "20O", "d", 0, 1}};
    phase->Print();
    // Sequential decay
    auto* seqgen {new ActSim::KinematicGenerator {"20O", "d", "d", "20O"}};
    seqgen->Print();
    ActPhysics::Particle deuton {"d"};
    auto bindingenergy {deuton.GetBE()};
    ActPhysics::Particle proton {"p"};
    ActPhysics::Particle neutron {"n"};
    // Kinary kinematics to reconstruct
    auto* reckin {new ActPhysics::Kinematics {"20O", "p", "p"}};


    // Energy losses
    auto* srim {new ActPhysics::SRIM()};
    srim->ReadTable("light",
                    TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", "1H").Data());
    srim->ReadTable("lightInSil",
                    TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_silicon.txt", "1H").Data());
    srim->ReadTable("beam",
                    TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", "20O").Data());

    // Silicon specs
    auto* specs {new ActPhysics::SilSpecs};
    specs->ReadFile("/media/Data/E796v2/configs/detailedSilicons.conf");
    auto* sm {new ActPhysics::SilMatrix {}};
    sm->Read("/media/Data/E796v2/Macros/SilVetos/Outputs/side_matrix.root");
    // auto silCentre = sm->GetMeanZ({3, 4, 5});
    sm->MoveZTo(128 - 7.52, {3, 4, 5}); // Beam Z is at 128 mm. Silicons are 7.7 mm lower
    specs->GetLayer("l0").ReplaceWithMatrix(sm);
    specs->EraseLayer("f0");
    specs->EraseLayer("f1");

    // Histograms
    ROOT::RDF::TH1DModel mEx {"hEx", "Ex;E_{x} [MeV];Counts", 150, -30, 20};
    auto hExSeq {mEx.GetHistogram()};
    hExSeq->SetTitle("Ex sequential");
    auto hExPS {mEx.GetHistogram()};
    hExPS->SetTitle("Ex phase space");

    ROOT::RDF::TH2DModel mKin {"hKin", "Kinematics;#theta_{p} [#circ];E_{p} [MeV]", 300, 0, 180, 300, 0, 100};
    auto hKinSeq {mKin.GetHistogram()};
    hKinSeq->SetTitle("Sequential kin");
    auto hKinPhase {mKin.GetHistogram()};
    hKinPhase->SetTitle("Phase space kin");

    ROOT::RDF::TH1DModel mWeight {"hW", "Weight histogram;w;Coutns", 200, 0, 1};
    auto hWSeq {mWeight.GetHistogram()};
    hWSeq->SetTitle("Sequential weight");
    auto hWPhase {mWeight.GetHistogram()};
    hWPhase->SetTitle("Phase space weight");

    // Number of iterations
    int iter {static_cast<int>(4e7)};
    for(int i = 0; i < iter; i++)
    {
        // Vertex
        auto vertex {SampleVertex()};
        auto EbeamVertex {srim->SlowWithStraggling("beam", EBeam, vertex.X())};
        // Kinematics
        reckin->SetBeamEnergyAndEx(EbeamVertex, 0); // This is not important to reconstruct Ex
        // Generate
        //////////////////////////////////////////////
        // Phase space
        {
            phase->SetBeamAndExEnergies(EbeamVertex, 0);
            auto w {phase->Generate()};
            auto* lor {phase->GetLorentzVector(1)};
            auto T3Lab {lor->E() - ActPhysics::Constants::kpMass};
            auto theta3Lab {lor->Theta()};
            auto phi3Lab {lor->Phi()};

            // Apply cuts
            if(ReachesSilicon(specs, srim, vertex, T3Lab, theta3Lab, phi3Lab))
            {
                hKinPhase->Fill(theta3Lab * TMath::RadToDeg(), T3Lab, w);
                hWPhase->Fill(w);
                // Reconstruct
                auto Ex {reckin->ReconstructExcitationEnergy(T3Lab, theta3Lab)};
                hExPS->Fill(Ex, w);
            }
        }
        //////////////////////////////////////////////
        // Sequential
        {
            seqgen->SetBeamAndExEnergies(EbeamVertex, 0);
            auto w {seqgen->Generate()};
            // Outgoing deuton
            auto* lor {seqgen->GetLorentzVector(0)};
            auto Td {lor->E() - deuton.GetGSMass()};
            auto thetad {lor->Theta()};
            auto phid {lor->Phi()};
            // And now decay to p + n
            deuton.SetEx(bindingenergy * 2);
            ActSim::DecayGenerator decay {deuton, proton, neutron};
            decay.SetDecay(Td, thetad, phid);
            w *= decay.Generate();
            // And owerwrite values
            lor = decay.GetLorentzVector(0);
            auto Tp {lor->E() - ActPhysics::Constants::kpMass};
            auto thetap {lor->Theta()};
            auto phip {lor->Phi()};

            if(ReachesSilicon(specs, srim, vertex, Tp, thetap, phip))
            {
                hKinSeq->Fill(thetap * TMath::RadToDeg(), Tp, w);
                hWSeq->Fill(w);
                // Reconstruct
                auto Ex {reckin->ReconstructExcitationEnergy(Tp, thetap)};
                hExSeq->Fill(Ex, w);
            }
        }
    }

    // Theoretical lines
    seqgen->SetBeamEnergy(EBeam);
    auto* gdd {seqgen->GetBinaryKinematics()->GetKinematicLine3()};
    gdd->SetLineColor(8);
    reckin->SetBeamEnergy(EBeam);
    auto* gpp {reckin->GetKinematicLine3()};
    gpp->SetLineColor(9);

    // Get other ps
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"SimulationTTree", "../../Outputs/juan_RPx/tree_20O_2H_2H_0.00_nPS_-1_pPS_0.root"};
    auto hSim {df.Histo1D(mEx, "Eex")};

    // Draw
    auto* c0 {new TCanvas {"c0", "Breakup testing canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hExSeq->SetLineColor(8);
    hExSeq->DrawNormalized("histe");
    hExPS->DrawNormalized("histe same");
    hSim->SetLineColor(kRed);
    hSim->DrawNormalized("histe same");
    gPad->BuildLegend();
    c0->cd(2);
    hKinPhase->DrawClone("colz");
    gdd->Draw("l");
    gpp->Draw("l");
    c0->cd(3);
    hKinSeq->DrawClone("colz");
    gdd->Draw("l");
    gpp->Draw("l");

    // Save results
    auto file {std::make_unique<TFile>("./Outputs/basic_simu.root", "update")};
    c0->Write();
}
