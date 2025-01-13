#include "ActColors.h"
#include "ActCutsManager.h"
#include "ActDecayGenerator.h"
#include "ActKinematicGenerator.h"
#include "ActKinematics.h"
#include "ActParticle.h"
#include "ActRunner.h"
#include "ActSRIM.h"
#include "ActSilMatrix.h"
#include "ActSilSpecs.h"
#include "ActTPCParameters.h"

#include "Rtypes.h"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TH2.h"
#include "TMath.h"
#include "TProfile2D.h"
#include "TROOT.h"
#include "TRandom.h"
#include "TRandom3.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TTree.h"

#include "Math/Point3Dfwd.h"
#include "Math/Vector3Dfwd.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include "/media/Data/E796v2/PostAnalysis/Gates.cxx"
#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"
#include "/media/Data/E796v2/Selector/Selector.h"

using XYZPoint = ROOT::Math::XYZPoint;
using XYZPointF = ROOT::Math::XYZPointF;
using XYZVector = ROOT::Math::XYZVector;
using XYZVectorF = ROOT::Math::XYZVectorF;

void ApplyNaN(double& val, double thresh = 0, const std::string& comment = "stopped")
{
    if(val <= thresh)
        val = std::nan(comment.c_str());
}

TH1D* GetRPXProj(const TString& file)
{
    auto* f {new TFile {file}};
    auto* p {f->Get<TH1D>("px")};
    if(!p)
        throw std::runtime_error("Simulation_E796::GetRPXProj(): could not get RPx projection in file " + file);
    p->SetDirectory(nullptr);
    delete f;
    return p;
}

double SampleXFromRPXProj(TH1D* px)
{
    return px->GetRandom();
}

std::pair<XYZPoint, XYZPoint> SampleVertex(double meanZ, double sigmaZ, TH3D* h, double lengthX)
{

    // X is always common for both manners
    double Xstart {0};
    double Xrp {gRandom->Uniform() * lengthX};
    // Y depends completely on the method of calculation
    double Ystart {-1};
    double Yrp {-1};
    // Z of beam at entrance
    double Zstart {gRandom->Gaus(meanZ, sigmaZ)};
    double Zrp {-1};
    // Ystart in this case is sampled from the histogram itself!
    double thetaXY {};
    double thetaXZ {};
    h->GetRandom3(Ystart, thetaXY, thetaXZ);
    // Mind that Y is not centred in the histogram value!
    // Rp values are computed as follows:
    Yrp = Ystart - Xrp * TMath::Tan(thetaXY * TMath::DegToRad());
    Zrp = Zstart - Xrp * TMath::Tan(thetaXZ * TMath::DegToRad());
    XYZPoint start {Xstart, Ystart, Zstart};
    XYZPoint vertex {Xrp, Yrp, Zrp};
    return {std::move(start), std::move(vertex)};
}

void ApplySilRes(double& e, double sigma)
{
    e = gRandom->Gaus(e, sigma * TMath::Sqrt(e / 5.5));
}

double AngleWithNormal(const ROOT::Math::XYZVector& dir, const ROOT::Math::XYZVector& normal)
{
    auto dot {dir.Unit().Dot(normal.Unit())};
    return TMath::ACos(dot);
}

void Simulation_E796(const std::string& beam, const std::string& target, const std::string& arglight, int neutronPS,
                     int protonPS, double T1, double Ex, bool standalone)
{
    // set batch mode if not an independent function
    if(!standalone)
        gROOT->SetBatch(true);

    // Set whether is elastic or not
    const bool isEl {target == arglight};

    // Set deuteron breakup type and if enabled (deutonbreaup != 0)
    int deutonbreakup {};
    if(neutronPS < 0)
        deutonbreakup = -1 * neutronPS;

    // Resolutions
    const double sigmaSil {0.060 / 2.355};
    const double sigmaPercentBeam {0.008};
    const double sigmaAngleLight {0.95 / 2.355};
    // Parameters of beam in mm
    // Center in Z
    // Mean is defined from silicon matrices
    const double zVertexSigma {3.73};
    // Center in Y
    const double yVertexMean {126.5}; // according to juan's emittance
    const double yVertexSigma {2.1};

    // Silicon thresholds
    const double thresholdSi0 {1.};
    const double thresholdSi1 {1.};

    // number of iterations
    // const int iterations {static_cast<int>((isEl) ? (neutronPS ? 1e8 : 5e7) : 3e7)};
    const int iterations {static_cast<int>(standalone ? 1e7 : 5e7)};

    // Which parameters will be activated
    bool stragglingInGas {true};
    bool stragglingInSil {true};
    bool silResolution {true};
    bool thetaResolution {true};

    // Silicon specs
    auto* specs {new ActPhysics::SilSpecs};
    specs->ReadFile("../configs/detailedSilicons.conf");
    // Silicon EFFECTIVE matrix
    auto* sm {E796Utils::GetEffSilMatrix(target, arglight)};
    // Set reference position and offset along Z!
    double silCentre {};
    double beamOffset {}; // determined from emittance calculations
    // Set layers
    std::string firstLayer {};
    std::string secondLayer {};
    if(isEl)
    {
        silCentre = sm->GetMeanZ({3, 4, 5});
        specs->GetLayer("l0").ReplaceWithMatrix(sm);
        beamOffset = 7.52; // mm
        // specs->GetLayer("l0").SetPoint({0, 174.425, 0});
        specs->EraseLayer("f0");
        specs->EraseLayer("f1");
        firstLayer = "l0";
    }
    else
    {
        silCentre = specs->GetLayer("f0").MeanZ({3, 4});
        beamOffset = 9.01; // mm
        sm->MoveZTo(silCentre, {3, 4});
        specs->EraseLayer("l0");
        firstLayer = "f0";
        secondLayer = "f1";
    }
    double zVertexMean {silCentre + beamOffset};

    // TPC basic parameters
    ActRoot::TPCParameters tpc {"Actar"};

    // Vertex sampling
    auto beamfile {std::make_unique<TFile>("/media/Data/E796v2/Macros/Emittance/Outputs/histos.root")};
    auto* hBeam {beamfile->Get<TH3D>("h3d")};
    if(!hBeam)
        throw std::runtime_error("Simulation_E796(): Could not load beam emittance histogram");
    hBeam->SetDirectory(nullptr);
    beamfile.reset();

    // Kinematics
    ActPhysics::Particle p1 {beam};
    ActPhysics::Particle p2 {target};
    ActPhysics::Particle p3 {arglight};
    // Automatically compute 4th particle
    ActPhysics::Kinematics kaux {p1, p2, p3};
    ActPhysics::Particle p4 {kaux.GetParticle(4)};
    // Binary kinematics generator
    ActSim::KinematicGenerator kingen {p1, p2, p3, p4, protonPS, (neutronPS > 0 ? neutronPS : 0)};
    kingen.Print();
    // Allow breakup of deuteron!
    ActSim::DecayGenerator decaygen;
    ActPhysics::Kinematics breakkin;
    // Set pointers to correct kinematics and define light particle to propagate in gas and sil
    auto light {arglight};
    ActPhysics::Kinematics* reckin {};
    if(deutonbreakup)
    {
        if(deutonbreakup == 1) // reconstructed as 20O(p,p)
        {
            decaygen = ActSim::DecayGenerator {"d", "p", "n"};
            breakkin = ActPhysics::Kinematics {beam, "p", "p"};
            light = "1H";
        }
        else
            throw std::invalid_argument("Simulation_E796(): only deutonbreakup == 1 is valid");
        decaygen.Print();
        reckin = &breakkin;
        std::cout << BOLDYELLOW << "Overriding light from " << arglight << " to " << light << RESET << '\n';
    }
    else
    {
        reckin = kingen.GetBinaryKinematics();
    }

    // CUTS ON SILICON ENERGY, depending on particle
    // from the graphical PID cut
    ActRoot::CutsManager<std::string> cuts;
    cuts.ReadCut(light, TString::Format("/media/Data/E796v2/PostAnalysis/Cuts/LightPID/pid_%s%s.root", light.c_str(),
                                        (isEl) ? "_side" : "")
                            .Data());
    std::pair<double, double> eLoss0Cut;
    if(cuts.GetCut(light))
    {
        eLoss0Cut = cuts.GetXRange(light);
        std::cout << BOLDGREEN << "-> ESil range for " << light << ": [" << eLoss0Cut.first << ", " << eLoss0Cut.second
                  << "] MeV" << RESET << '\n';
    }
    else
    {
        std::cout << BOLDRED << "Simulation_E796(): could not read PID cut for " << light
                  << " -> using default eLoss0Cut" << RESET << '\n';
        eLoss0Cut = {0, 1000};
    }

    //---- SIMULATION STARTS HERE
    ROOT::EnableImplicitMT();

    // timer
    TStopwatch timer {};
    timer.Start();

    // Histograms
    // To compute a fine-grain efficiency, we require at least a binning width of 0.25 degrees!
    auto hThetaCM {HistConfig::ThetaCM.GetHistogram()};
    auto hThetaCMAll {HistConfig::ChangeTitle(HistConfig::ThetaCM, "ThetaCM all", "All").GetHistogram()};
    auto hDistF0 {HistConfig::ChangeTitle(HistConfig::TL, "Distance to F0").GetHistogram()};
    auto hKinVertex {HistConfig::ChangeTitle(HistConfig::KinSimu, "Kinematics at vertex").GetHistogram()};
    auto hSP {HistConfig::SP.GetHistogram()};
    auto hEexAfter {HistConfig::ChangeTitle(HistConfig::Ex, "Ex after resolutions").GetHistogram()};
    auto hSPTheta {std::make_unique<TProfile2D>("hSPTheta", "SP vs #theta_{CM};Y [mm];Z [mm];#theta_{CM} [#circ]", 75,
                                                0, 300, 75, 0, 300)};
    auto hRP {HistConfig::RP.GetHistogram()};
    auto hRPz {std::make_unique<TH2D>("hRPz", "RP;Y [mm];Z [mm]", 550, 0, 256, 550, 0, 256)};
    // Debug histograms
    auto hDeltaE {
        std::make_unique<TH2D>("hDeltaEE", "#Delta E - E;E_{in} [MeV];#Delta E_{0} [MeV]", 300, 0, 60, 300, 0, 60)};
    auto hELoss0 {std::make_unique<TH2D>("hELoss0", "ELoss0;E_{in} [MeV];#Delta E_{0} [MeV]", 200, 0, 40, 200, 0, 40)};

    // Load SRIM tables
    // The name of the file sets particle + medium
    auto* srim {new ActPhysics::SRIM()};
    srim->ReadTable(
        "light",
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", light.c_str()).Data());
    srim->ReadTable(
        "beam",
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", beam.c_str()).Data());
    srim->ReadTable(
        "lightInSil",
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_silicon.txt", light.c_str()).Data());


    // Random generator
    gRandom->SetSeed();
    // Runner: contains utility functions to execute multiple actions
    ActSim::Runner runner(nullptr, nullptr, gRandom, 0);

    // Output from simulation!
    // We only store a few things in the TTree
    // 1-> Excitation energy
    // 2-> Theta in CM frame
    // 3-> Weight of the generator: for three-body reactions (phase spaces) the other two
    // variables need to be weighted by this value. For binary reactions, weight = 1
    // 4-> Energy at vertex
    // 5-> Theta in Lab frame
    auto* outFile {new TFile(gSelector->GetSimuFile(beam, target, arglight, Ex, neutronPS, protonPS), "recreate")};
    auto* outTree {new TTree("SimulationTTree", "A TTree containing only our Eex obtained by simulation")};
    double theta3CM_tree {};
    outTree->Branch("theta3CM", &theta3CM_tree);
    double Ex_tree {};
    outTree->Branch("Eex", &Ex_tree);
    double weight_tree {};
    outTree->Branch("weight", &weight_tree);
    double EVertex_tree {};
    outTree->Branch("EVertex", &EVertex_tree);
    double theta3Lab_tree {};
    outTree->Branch("theta3Lab", &theta3Lab_tree);
    double rpx_tree {};
    outTree->Branch("RPx", &rpx_tree);

    // RUN!
    // print fancy info
    std::cout << BOLDMAGENTA << "Running for Ex = " << Ex << " MeV" << RESET << '\n';
    std::cout << BOLDGREEN;
    const int percentPrint {5};
    int step {iterations / (100 / percentPrint)};
    int nextPrint {step};
    int percent {};
    for(long int reaction = 0; reaction < iterations; reaction++)
    {
        // Print progress
        if(reaction >= nextPrint)
        {
            percent = 100 * (reaction + 1) / iterations;
            int nchar {percent / percentPrint};
            std::cout << "\r" << std::string((int)(percent / percentPrint), '|') << percent << "%";
            std::cout.flush();
            nextPrint += step;
        }
        // 1-> Sample vertex
        auto [start, vertex] {SampleVertex(zVertexMean, zVertexSigma, hBeam, tpc.X())};

        // 2-> Beam energy according to its sigma
        auto TBeam {runner.RandomizeBeamEnergy(
            T1 * p1.GetAMU(),
            sigmaPercentBeam * T1 * p1.GetAMU())}; // T1 in Mev / u * mass of beam in u = total kinetic energy
        // And slow according to distance travelled
        auto distToVertex {(vertex - start).R()};
        TBeam = srim->Slow("beam", TBeam, distToVertex);

        // 3-> Run kinematics!
        kingen.SetBeamAndExEnergies(TBeam, Ex);
        double weight {kingen.Generate()};
        // focus on recoil 3 (light)
        auto* PLight {kingen.GetLorentzVector(0)};
        auto theta3Lab {PLight->Theta()};
        auto phi3Lab {PLight->Phi()};
        auto T3Lab {PLight->Energy() - p3.GetMass()};
        // If breakup, override values
        if(deutonbreakup)
        {
            decaygen.SetDecay(T3Lab, theta3Lab, phi3Lab);
            auto bw {decaygen.Generate()};
            auto* proton {decaygen.GetLorentzVector(0)};
            theta3Lab = proton->Theta();
            phi3Lab = proton->Phi();
            T3Lab = (proton->E() - decaygen.GetFinalMass(0));
            breakkin.SetBeamEnergy(TBeam);
        }
        // Simualated thetaCM
        double thetaCMBefore {reckin->ReconstructTheta3CMFromLab(T3Lab, theta3Lab)};
        hThetaCMAll->Fill(thetaCMBefore * TMath::RadToDeg());

        // 4-> Include thetaLab resolution to compute thetaCM and Ex afterwards
        if(thetaResolution) // resolution in
            theta3Lab = gRandom->Gaus(theta3Lab, sigmaAngleLight * TMath::DegToRad());

        // 4.1 -> Apply cut on vertex position
        if(!E796Gates::rp(vertex.X()))
            continue;

        // 5-> Propagate track from vertex to silicon wall using SilSpecs class
        // And using the angle with the uncertainty already in
        ROOT::Math::XYZVector dirBeamFrame {TMath::Cos(theta3Lab), TMath::Sin(theta3Lab) * TMath::Sin(phi3Lab),
                                            TMath::Sin(theta3Lab) * TMath::Cos(phi3Lab)};
        // Declare beam direction
        auto beamDir {(vertex - start).Unit()};
        // Rotate to world = geometry frame
        auto dirWorldFrame {runner.RotateToWorldFrame(dirBeamFrame, beamDir)};
        auto [silIndex0, silPoint0InMM] {specs->FindSPInLayer(firstLayer, vertex, dirWorldFrame)};

        // skip tracks that doesn't reach silicons or are not in SiliconMatrix indexes
        if(silIndex0 == -1 || !(sm->IsInMatrix(silIndex0)))
            continue;

        // Apply SilMatrix cut
        if(!sm->IsInside(silIndex0, (isEl ? silPoint0InMM.X() : silPoint0InMM.Y()), silPoint0InMM.Z()))
            continue;
        // And if elastic, apply cut in silicon index
        if(isEl)
            if(!E796Gates::maskelsil(silIndex0))
                continue;

        // Define SP distance
        auto distance0 {(vertex - silPoint0InMM).R()};
        auto T3EnteringSil {srim->SlowWithStraggling("light", T3Lab, distance0)};
        ApplyNaN(T3EnteringSil);
        // nan if stopped in gas
        if(!std::isfinite(T3EnteringSil))
            continue;

        // First layer of silicons
        // Angle with normal
        auto angleNormal0 {AngleWithNormal(dirWorldFrame, {(isEl ? 0. : 1.), (isEl ? 1. : 0.), 0})};
        auto T3AfterSil0 {srim->SlowWithStraggling("lightInSil", T3EnteringSil,
                                                   specs->GetLayer(firstLayer).GetUnit().GetThickness(), angleNormal0)};
        auto eLoss0 {T3EnteringSil - T3AfterSil0};
        // Apply resolution
        if(T3AfterSil0 != 0)
        {
            ApplySilRes(eLoss0, sigmaSil);
            T3AfterSil0 = T3EnteringSil - eLoss0;
        }
        ApplyNaN(eLoss0, thresholdSi0, "thresh");
        // nan if bellow threshold
        if(!std::isfinite(eLoss0))
            continue;
        hDeltaE->Fill(T3EnteringSil, eLoss0);

        // 6-> Same but to silicon layer 1 if exists
        double T3AfterInterGas {};
        double distance1 {};
        int silIndex1 {};
        ROOT::Math::XYZPoint silPoint1 {};
        double eLoss1 {};
        double T3AfterSil1 {};
        bool isPunch {};
        if(T3AfterSil0 > 0 && !isEl)
        {
            // first, propagate in gas
            auto [silIndex1, silPoint1InMM] {specs->FindSPInLayer(secondLayer, vertex, dirWorldFrame)};
            if(silIndex1 == -1)
                continue;

            distance1 = (silPoint1InMM - silPoint0InMM).R();
            T3AfterInterGas = srim->SlowWithStraggling("light", T3AfterSil0, distance1);
            ApplyNaN(T3AfterInterGas);
            if(!std::isfinite(T3AfterInterGas))
                continue;

            // now, silicon if we have energy left
            if(T3AfterInterGas > 0)
            {
                // For e796 angleNormal0 = angleNormal1 but this is not general
                auto angleNormal1 {angleNormal0};
                auto T3AfterSil1 {srim->SlowWithStraggling("lightInSil", T3AfterInterGas,
                                                           specs->GetLayer(secondLayer).GetUnit().GetThickness(),
                                                           angleNormal1)};
                auto eLoss1 {T3AfterInterGas - T3AfterSil1};
                ApplySilRes(eLoss1, sigmaSil);
                T3AfterSil1 = T3AfterInterGas - eLoss1;
                ApplyNaN(eLoss1, thresholdSi1, "thresh");
                isPunch = true;
            }
        }

        // 7 -> Reconstruct energy at vertex
        double EBefSil0 {};
        if(isPunch && T3AfterSil1 == 0 && std::isfinite(eLoss1))
        {
            double EAfterSil0 {srim->EvalInitialEnergy("light", eLoss1, distance1)};
            EBefSil0 = eLoss0 + EAfterSil0;
        }
        else if(!isPunch && T3AfterSil0 == 0)
            EBefSil0 = eLoss0;
        else
            EBefSil0 = -1;

        // 7->
        // we are ready to reconstruct Eex with all resolutions implemented
        //(d,light) is investigated gating on Esil1 = 0!
        // bool cutEAfterSil0 {EBefSil0 != -1 && !isPunch};
        bool cutEAfterSil0 {T3AfterSil0 == 0};
        bool cutELoss0 {eLoss0Cut.first <= eLoss0 && eLoss0 <= eLoss0Cut.second};
        if(cutEAfterSil0 && cutELoss0) // fill histograms
        {
            auto T3Recon {srim->EvalInitialEnergy("light", EBefSil0, distance0)};
            auto ExAfter {reckin->ReconstructExcitationEnergy(T3Recon, theta3Lab)};
            auto thetaCM {reckin->ReconstructTheta3CMFromLab(T3Recon, theta3Lab)};

            // fill histograms
            hThetaCM->Fill(thetaCM * TMath::RadToDeg());
            hDistF0->Fill(distance0);
            hKinVertex->Fill(theta3Lab * TMath::RadToDeg(), T3Recon);
            hEexAfter->Fill(ExAfter, weight);
            hSP->Fill((isEl) ? silPoint0InMM.X() : silPoint0InMM.Y(), silPoint0InMM.Z());
            // Fill histogram of SP with thetaCM as weight
            hSPTheta->Fill(silPoint0InMM.Y(), silPoint0InMM.Z(), thetaCM * TMath::RadToDeg());
            // RP histogram
            hRP->Fill(vertex.X(), vertex.Y());
            hRPz->Fill(isEl ? vertex.X() : vertex.Y(), vertex.Z());
            hELoss0->Fill(T3EnteringSil, eLoss0);

            // write to TTree
            Ex_tree = ExAfter;
            weight_tree = weight;
            theta3CM_tree = thetaCM * TMath::RadToDeg();
            EVertex_tree = T3Recon;
            theta3Lab_tree = theta3Lab * TMath::RadToDeg();
            rpx_tree = vertex.X();
            outTree->Fill();
        }
    }
    std::cout << "\r" << std::string(100 / percentPrint, '|') << 100 << "%";
    std::cout.flush();
    std::cout << RESET << '\n';

    // Efficiencies as quotient of histograms in TEfficiency class
    auto* eff {new TEfficiency(*hThetaCM, *hThetaCMAll)};
    eff->SetNameTitle("eff", TString::Format("#theta_{CM} eff E_{x} = %.2f MeV", Ex));

    // SAVING
    outFile->cd();
    outTree->Write();
    eff->Write();
    hSP->Write("hSP");
    hRP->Write("hRP");
    outFile->Close();
    delete outFile;
    outFile = nullptr;

    // plotting
    if(standalone)
    {
        auto* c0 {new TCanvas("c0", "Canvas for inspection 0")};
        c0->DivideSquare(6);
        c0->cd(1);
        hThetaCM->DrawClone();
        c0->cd(2);
        hThetaCMAll->DrawClone();
        c0->cd(3);
        hDistF0->DrawClone();
        c0->cd(4);
        // hThetaCMAll->DrawClone();
        hRP->DrawClone("colz");
        c0->cd(5);
        hRPz->DrawClone("colz");
        sm->DrawClone();
        c0->cd(6);
        hELoss0->DrawClone("colz");

        // draw theoretical kinematics
        ActPhysics::Kinematics theokin {p1, p2, p3, p4, T1 * p1.GetAMU(), Ex};
        if(neutronPS == 1)
            theokin.SetEx(p4.GetSn());
        if(neutronPS == 2)
            theokin.SetEx(p4.GetS2n());
        auto* gtheo {theokin.GetKinematicLine3()};

        auto* c1 {new TCanvas("cAfter", "Canvas for inspection 1")};
        c1->DivideSquare(6);
        c1->cd(1);
        hKinVertex->DrawClone("colz");
        gtheo->Draw("same");
        c1->cd(2);
        hSP->DrawClone("col");
        if(sm)
            sm->Draw();
        c1->cd(3);
        hEexAfter->DrawClone("hist");
        c1->cd(4);
        eff->Draw("apl");
        c1->cd(5);
        hSPTheta->DrawClone("colz");
        c1->cd(6);
        hDeltaE->DrawClone("colz");

        // hRPx->DrawNormalized();
        // hRPxSimu->SetLineColor(kRed);
        // hRPxSimu->DrawNormalized("same");
    }

    // deleting news
    delete srim;

    timer.Stop();
    timer.Print();
}
