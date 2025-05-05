#ifndef simu_cxx
#define simu_cxx

#include "ActKinematics.h"
#include "ActLine.h"
#include "ActSRIM.h"
#include "ActSilSpecs.h"
#include "ActTPCParameters.h"
#include "ActUtils.h"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TMath.h"
#include "TRandom.h"
#include "TString.h"
#include "TTree.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#include <iostream>
#include <string>

using XYZPoint = ROOT::Math::XYZPoint;
using XYZVector = ROOT::Math::XYZVector;

XYZPoint SampleVertex(ActRoot::TPCParameters* tpc)
{
    auto x {gRandom->Uniform(0, tpc->X())};
    auto y {gRandom->Gaus(tpc->Y() / 2, 2)};
    auto z {gRandom->Gaus(tpc->Z() / 2, 2)};
    return {x, y, z};
}

void simu(const std::string& beam, const std::string& target, const std::string& light, double Tbeam, double Ex)
{
    // Uncertainties
    double angleSigma {1 / 2.35 * TMath::DegToRad()};
    // Thresholds
    double silThresh {0.5}; // MeV

    // Load TPC
    ActRoot::TPCParameters tpc {"Actar"};

    // Load silicons
    ActPhysics::SilSpecs sils;
    sils.ReadFile("./Inputs/silicons.conf");
    // Center silicons on beam
    sils.GetLayer("f0").MoveZTo(tpc.Z() / 2, {7});
    sils.GetLayer("f1").MoveZTo(tpc.Z() / 2, {7});

    // Kinematics
    auto* kin = new ActPhysics::Kinematics {beam, target, light, Tbeam, Ex};

    // SRIM
    auto* srim {new ActPhysics::SRIM};
    std::string path {"./Inputs/SRIM/"};
    std::string filetag {"_900mb_CF4_90-10.txt"};
    srim->ReadTable("beam", path + beam + filetag);
    srim->ReadTable("light", path + light + filetag);
    srim->ReadTable("lightInSil", path + light + "_silicon.txt");

    // Histograms
    auto* hRP {new TH2D {"hRP", "RP;X [mm];Y [mm]", 300, 0, 260, 300, 0, 260}};
    auto* hSP {new TH2D {"hSP", "SP;Y [mm];Z [mm]", 300, 0, 260, 300, 0, 260}};
    auto* hThetaCMAll {new TH1D {"hCMAll", "CM all;#theta_{CM} [#circ]", 300, 0, 180}};
    auto* hThetaCMIn {(TH1D*)hThetaCMAll->Clone("hThetaCMIn")};
    auto* hKin {new TH2D {"hKin", "Sampled kinematics;#theta_{Lab} [#circ];T_{Lab} [MeV]", 300, 0, 90, 300, 0, 60}};
    auto* hKinIn {(TH2D*)hKin->Clone("hKinIn")};
    hKinIn->SetTitle("In kinematics");
    auto* hGas0 {new TH2D {"hGas0", "PID gas vs 0;#DeltaE_{Sil0} [MeV];#DeltaE_{gas} [MeV]", 600, 0, 40, 800, 0, 2}};
    auto* hdE01 {
        new TH2D {"hdE01", "#Delta E 0 to 1;#DeltaE_{Sil0} [MeV];#DeltaE_{Sil1} [MeV]", 300, 0, 40, 300, 0, 40}};
    auto* hRec {new TH2D {"hRec", "Rec initial energy;Simu;SRIM", 400, 0, 100, 400, 0, 100}};


    // Saving in file
    auto* oufile {new TFile {
        TString::Format("./Outputs/tree_%s_%s_%s_Ex_%.2f.root", beam.c_str(), target.c_str(), light.c_str(), Ex),
        "recreate"}};
    auto* tree {new TTree {"SimulationTree", "A simple simulation Tree"}};
    double dEgas_tree {};
    tree->Branch("dEgas", &dEgas_tree);
    double dE0_tree {};
    tree->Branch("dE0", &dE0_tree);
    double dE1_tree {};
    tree->Branch("dE1", &dE1_tree);
    double t3_tree {};
    tree->Branch("T3", &t3_tree);
    double theta3_tree {};
    tree->Branch("theta3", &theta3_tree);
    double rpx_tree {};
    tree->Branch("RPx", &rpx_tree);
    double t3after1_tree {};
    tree->Branch("T3After1", &t3after1_tree);


    // Number of iterations
    const auto iter {static_cast<int>(5e6)};

    for(int i = 0; i < iter; i++)
    {
        // 1-> Sample vertex
        auto vertex {SampleVertex(&tpc)};
        hRP->Fill(vertex.X(), vertex.Y());
        // 2-> Slow down beam
        auto TbeamIter {srim->SlowWithStraggling("beam", Tbeam, vertex.X())};
        // 3-> Kinematics
        kin->SetBeamEnergy(TbeamIter);
        auto thetaCM {TMath::ACos(gRandom->Uniform(-1, 1))};
        hThetaCMAll->Fill(thetaCM * TMath::RadToDeg());
        auto phiCM {gRandom->Uniform(0, TMath::TwoPi())};
        kin->ComputeRecoilKinematics(thetaCM, phiCM);
        auto T3 {kin->GetT3Lab()};
        auto theta3Lab {kin->GetTheta3Lab()};
        // Apply resolution on angle
        theta3Lab = gRandom->Gaus(theta3Lab, angleSigma);
        auto phi3Lab {kin->GetPhi3Lab()};
        hKin->Fill(theta3Lab * TMath::RadToDeg(), T3);

        // Direction of light particle
        XYZVector dir {std::cos(theta3Lab), std::sin(theta3Lab) * std::sin(phi3Lab),
                       std::sin(theta3Lab) * std::cos(phi3Lab)};

        // Check it reaches silicons
        auto [silIdx, silPoint0] {sils.FindSPInLayer("f0", vertex, dir)};
        if(silIdx == -1) // no impact on silicon
            continue;
        hSP->Fill(silPoint0.Y(), silPoint0.Z());

        // 4-> Propagation to silicon0
        auto T3At0 {srim->SlowWithStraggling("light", T3, (silPoint0 - vertex).R())};
        auto eGas {T3 - T3At0};
        // And divide by track length inside actar tpc pad plane
        ActRoot::Line line {ActRoot::CastXYZPoint<float>(vertex), ActRoot::CastXYZVector<float>(dir), -1};
        // auto trackLength {(line.MoveToX(256.) - vertex).R()};
        auto trackLength {1};
        eGas /= trackLength;

        if(T3At0 <= 0)
            continue;
        XYZVector silNormal {1, 0, 0};
        auto thetaSil {TMath::ACos(silNormal.Dot(dir.Unit()))};
        auto T3After0 {
            srim->SlowWithStraggling("lightInSil", T3At0, sils.GetLayer("f0").GetUnit().GetThickness(), thetaSil)};
        auto eLoss0 {T3At0 - T3After0};
        if(eLoss0 < silThresh)
            continue;

        // 5-> Propagation to silicon1
        double T3At1 {};
        double T3After1 {};
        double eLoss1 {};
        if(T3After0 > 0)
        {
            // Propagate
            auto [_, silPoint1] {sils.FindSPInLayer("f1", vertex, dir)};
            auto distIntergas {(silPoint1 - silPoint0).R()};
            T3At1 = srim->SlowWithStraggling("light", T3After0, distIntergas);
            if(T3At1 > 0)
            {
                // ELoss in silicon
                T3After1 = srim->SlowWithStraggling("lightInSil", T3At1, sils.GetLayer("f1").GetUnit().GetThickness(),
                                                    thetaSil);
                eLoss1 = T3At1 - T3After1;
                if(eLoss1 < silThresh)
                    continue;
            }
            else
            {
                continue;
            }
        }
        // Fill Histograms
        if(T3After1 <= 0)
        {
            hKinIn->Fill(theta3Lab * TMath::RadToDeg(), T3);
            hThetaCMIn->Fill(thetaCM * TMath::RadToDeg());
        }
        if(T3After1 > 0)
        {
            auto rec {srim->EvalInitialEnergyFromDeltaE("lightInSil", eLoss1,
                                                        sils.GetLayer("f1").GetUnit().GetThickness(), thetaSil)};
            hRec->Fill(T3At1, rec);
            hKinIn->Fill(theta3Lab * TMath::RadToDeg(), T3);
        }
        hGas0->Fill(eLoss0, eGas);
        hdE01->Fill(eLoss0, eLoss1);
        // Fill tree
        dEgas_tree = eGas;
        dE0_tree = eLoss0;
        dE1_tree = eLoss1;
        t3_tree = T3;
        theta3_tree = theta3Lab * TMath::RadToDeg();
        rpx_tree = vertex.X();
        t3after1_tree = T3After1;
        tree->Fill();
    }

    // Efficiency
    auto* eff {new TEfficiency {*hThetaCMIn, *hThetaCMAll}};

    // Write
    oufile->cd();
    tree->Write();
    eff->Write("eff");

    // Draw
    auto* c0 {new TCanvas {"c0", "Simu canvas 0"}};
    c0->DivideSquare(8);
    c0->cd(1);
    hRP->Draw("colz");
    c0->cd(2);
    hSP->Draw("colz");
    sils.GetLayer("f0").GetSilMatrix()->DrawClone();
    c0->cd(3);
    hThetaCMAll->Draw();
    hThetaCMIn->SetLineColor(8);
    hThetaCMIn->Draw("same");
    c0->cd(4);
    hKinIn->Draw("colz");
    c0->cd(5);
    hGas0->Draw("colz");
    c0->cd(6);
    hdE01->Draw("colz");
    c0->cd(7);
    hRec->Draw("colz");
}
#endif
