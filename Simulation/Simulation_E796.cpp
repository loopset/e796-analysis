#include "ActColors.h"
#include "ActGeometry.h"
#include "ActKinematicGenerator.h"
#include "ActKinematics.h"
#include "ActParticle.h"
#include "ActRunner.h"
#include "ActSRIM.h"
#include "ActSilMatrix.h"

#include "Rtypes.h"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TMath.h"
#include "TProfile2D.h"
#include "TROOT.h"
#include "TRandom3.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TTree.h"

#include "Math/Point3Dfwd.h"

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
#include "/media/Data/E796v2/Simulation/Utils.cxx"

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

void Simulation_E796(const std::string& beam, const std::string& target, const std::string& light,
                     const std::string& heavy, int neutronPS, int protonPS, double T1, double Ex, bool standalone)
{
    // set batch mode if not an independent function
    if(!standalone)
        gROOT->SetBatch(true);

    // SIGMAS
    const double sigmaSil {0.060 / 2.355};
    const double sigmaPercentBeam {0.008};
    const double sigmaAngleLight {0.95 / 2.355};
    // Parameters of beam in mm
    // Beam has to be manually placed in the simulation
    // Centered in Z and Y with a width of 4 mm
    // Center in Z
    const double zVertexMean {128.};
    const double zVertexSigma {4};
    // Center in Y
    const double yVertexMean {128.};
    const double yVertexSigma {4};
    // const double zVertexMean {83.59};
    // const double zVertexSigma {3.79};

    // THRESHOLDS FOR SILICONS
    const double thresholdSi0 {1.};
    const double thresholdSi1 {1.};

    // number of iterations
    const int iterations {static_cast<int>(1e7)};

    // ACTIVATE STRAGGLING OR NOT
    bool stragglingInGas {true};
    bool stragglingInSil {true};
    bool silResolution {true};
    bool thetaResolution {true};

    // CUTS ON SILICON ENERGY, depending on particle
    std::pair<double, double> eLoss0Cut {0, 1000}; //{6.5, 27.};

    // Silicon masks
    auto sm {E796Utils::GetEffSilMatrix(light)};
    // auto silIndxs {sm.GetSilIndexes()};
    std::set<int> silIndxs {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    // Move Z of silicons to match centre of chamber. The centred silicons are 3 and 4
    sm->MoveZTo(zVertexMean, {3, 4});

    // RPx histogram to sample
    auto* hRPx {GetRPXProj(E796Utils::GetFileName(2, beam, target, light, false, "rpx"))};
    // // // histogram file
    // auto* histfile {new TFile("./Inputs/BeamY_ThetaXY_and_XZ_space_3Dhisto.root")};
    // auto* hBeam {histfile->Get<TH3F>("h_Y_thetaXY_thetaXZ")};
    // if(!hBeam)
    //     throw std::runtime_error("Could not load beam emittance histogram");
    // // closing hBeam file and coming back to main ROOT directory
    // gROOT->cd();
    // hBeam->SetDirectory(nullptr);
    // histfile->Close();
    // delete histfile;
    // histfile = nullptr;

    //---- SIMULATION STARTS HERE
    ROOT::EnableImplicitMT();

    // timer
    TStopwatch timer {};
    timer.Start();

    // Init particles
    ActPhysics::Particle p1 {beam};
    ActPhysics::Particle p2 {target};
    ActPhysics::Particle p3 {light};
    ActPhysics::Particle p4 {heavy};
    // Init kinematics generator
    ActSim::KinematicGenerator kingen {p1, p2, p3, p4, protonPS, neutronPS};
    kingen.Print();

    // Histograms
    // To compute a fine-grain efficiency, we require at least a binning width of 0.25 degrees!
    auto hThetaCM {HistConfig::ThetaCM.GetHistogram()};
    auto hThetaCMAll {HistConfig::ChangeTitle(HistConfig::ThetaCM, "ThetaCM all", "All").GetHistogram()};
    // Add additional histograms to compute eff per sections of actar
    auto hThetaCM1 {HistConfig::ChangeTitle(HistConfig::ThetaCM, "ThetaCM1").GetHistogram()};
    auto hThetaCMAll1 {HistConfig::ChangeTitle(HistConfig::ThetaCM, "ThetaCM all 1").GetHistogram()};
    auto hThetaCM2 {HistConfig::ChangeTitle(HistConfig::ThetaCM, "ThetaCM2").GetHistogram()};
    auto hThetaCMAll2 {HistConfig::ChangeTitle(HistConfig::ThetaCM, "ThetaCM all 2").GetHistogram()};

    auto hDistL0 {HistConfig::ChangeTitle(HistConfig::TL, "Distance to L0").GetHistogram()};

    auto hKinVertex {HistConfig::ChangeTitle(HistConfig::KinSimu, "Kinematics at vertex").GetHistogram()};

    auto hSP {HistConfig::SP.GetHistogram()};

    auto hEexBefore {HistConfig::ChangeTitle(HistConfig::Ex, "Ex before resolutions", "Bef").GetHistogram()};
    auto hEexAfter {HistConfig::ChangeTitle(HistConfig::Ex, "Ex after resolutions", "After").GetHistogram()};

    auto hSPTheta {std::make_unique<TProfile2D>("hSPTheta", "SP vs #theta_{CM};Y [mm];Z [mm];#theta_{CM} [#circ]", 75,
                                                0, 300, 75, 0, 300)};

    auto hRP {HistConfig::RP.GetHistogram()};

    auto* hRPxSimu {HistConfig::RP.GetHistogram()->ProjectionX("hRPxSimu")};

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

    // Load geometry
    auto* geometry {new ActSim::Geometry()};
    geometry->ReadGeometry("/media/Data/E796v2/Simulation/Geometries/", "geo0");

    // Random generator
    auto* rand {new TRandom3()};
    rand->SetSeed(); // random path in each execution of macro

    // Runner: contains utility funcstions to execute multiple actions
    ActSim::Runner runner(srim, geometry, rand, sigmaSil);

    // Output from simulation!
    // We only store a few things in the TTree
    // 1-> Excitation energy
    // 2-> Theta in CM frame
    // 3-> Weight of the generator: for three-body reactions (phase spaces) the other two
    // variables need to be weighted by this value. For binary reactions, weight = 1
    // 4-> Energy at vertex
    // 5-> Theta in Lab frame
    auto* outFile {new TFile(E796Simu::GetFile(beam, target, light, Ex, neutronPS, protonPS), "recreate")};
    auto* outTree {new TTree("SimulationTTree", "A TTree containing only our Eex obtained by simulation")};
    double theta3CM_tree {};
    outTree->Branch("theta3CM", &theta3CM_tree);
    double Eex_tree {};
    outTree->Branch("Eex", &Eex_tree);
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
        // 1-> Sample vertex and apply same cut as in analysis
        auto vertex {runner.SampleVertex(yVertexMean, yVertexSigma, zVertexMean, zVertexSigma, nullptr)};
        if(!E796Gates::rp(vertex.X()))
            continue;
        // 2-> Beam energy according to its sigma
        auto TBeam {runner.RandomizeBeamEnergy(
            T1 * p1.GetAMU(),
            sigmaPercentBeam * T1 * p1.GetAMU())}; // T1 in Mev / u * mass of beam in u = total kinetic energy
        // 3-> Run kinematics!
        kingen.SetBeamAndExEnergies(TBeam, Ex);
        double weight {kingen.Generate()};
        // focus on recoil 3 (light)
        // obtain thetas and energies
        auto* PLight {kingen.GetLorentzVector(0)};
        auto theta3Lab {PLight->Theta()};
        auto T3Lab {PLight->Energy() - p3.GetMass()};
        auto EexBefore {kingen.GetBinaryKinematics().ReconstructExcitationEnergy(T3Lab, theta3Lab)};
        // to compute geometric efficiency by CM interval and with our set reference direction
        double theta3CMBefore {TMath::Pi() - kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Lab, theta3Lab)};
        hThetaCMAll->Fill(theta3CMBefore * TMath::RadToDeg());
        // Divide per regions
        if(E796Gates::rpx1(vertex))
            hThetaCMAll1->Fill(theta3CMBefore * TMath::RadToDeg());
        if(E796Gates::rpx2(vertex))
            hThetaCMAll2->Fill(theta3CMBefore * TMath::RadToDeg());
        // 4-> Include thetaLab resolution to compute thetaCM and Ex
        if(thetaResolution) // resolution in
            theta3Lab = runner.GetRand()->Gaus(theta3Lab, sigmaAngleLight * TMath::DegToRad());
        auto phi3Lab {runner.GetRand()->Uniform(0., 2 * TMath::Pi())};

        // 5-> Propagate track from vertex to silicon wall using Geometry class
        ROOT::Math::XYZVector direction {TMath::Cos(theta3Lab), TMath::Sin(theta3Lab) * TMath::Sin(phi3Lab),
                                         TMath::Sin(theta3Lab) * TMath::Cos(phi3Lab)};
        auto vertexInGeoFrame {runner.DisplacePointToTGeometryFrame(vertex)};
        ROOT::Math::XYZPoint silPoint0 {};
        int silType0 {};
        int silIndex0 {};
        double distance0 {};
        bool side0 {};
        // Assembly 0
        int assemblyIndex {0};
        runner.GetGeo()->PropagateTrackToSiliconArray(vertexInGeoFrame, direction, assemblyIndex, side0, distance0,
                                                      silType0, silIndex0, silPoint0);
        // convert to mm (Geometry::PropagateTracksToSiliconArray works in cm but we need mm to use in SRIM)
        distance0 *= 10.;

        // skip tracks that doesn't reach silicons or are in silicon index cut
        if(silIndex0 == -1 || (silIndxs.find(silIndex0) == silIndxs.end()))
            continue;
        auto silPoint0InMM {runner.DisplacePointToStandardFrame(silPoint0)};
        // Apply SilMatrix cut
        if(!sm->IsInside(silIndex0, silPoint0InMM.Y(), silPoint0InMM.Z()))
            continue;

        auto T3EnteringSil {runner.EnergyAfterGas(T3Lab, distance0, "light", stragglingInGas)};
        // nan if stopped in gas
        if(!std::isfinite(T3EnteringSil))
            continue;

        // SILICON0
        auto [eLoss0,
              T3AfterSil0] {runner.EnergyAfterSilicons(T3EnteringSil, geometry->GetAssemblyUnitWidth(0) * 10.,
                                                       thresholdSi0, "lightInSil", silResolution, stragglingInSil)};
        // nan if bellow threshold
        if(!std::isfinite(eLoss0))
            continue;
        // 6-> Same but to silicon layer 1
        // SILICON1
        double T3AfterInterGas {};
        double distance1 {};
        int silIndex1 {};
        int silType1 {};
        bool side1 {};
        ROOT::Math::XYZPoint silPoint1 {};
        double eLoss1 {};
        double T3AfterSil1 {};
        if(T3AfterSil0 > 0)
        {
            // first, propagate in gas
            assemblyIndex = 1;
            runner.GetGeo()->PropagateTrackToSiliconArray(vertexInGeoFrame, direction, assemblyIndex, side1, distance1,
                                                          silType1, silIndex1, silPoint1, false);
            if(silIndex1 == -1)
                continue;

            distance1 *= 10.;
            T3AfterInterGas = runner.EnergyAfterGas(T3AfterSil0, distance1, "light", stragglingInGas);

            // now, silicon if we have energy left
            if(T3AfterInterGas > 0)
            {
                auto results {runner.EnergyAfterSilicons(T3AfterInterGas, geometry->GetAssemblyUnitWidth(1) * 10.,
                                                         thresholdSi1, "lightInSil", silResolution, stragglingInSil)};
                eLoss1 = results.first;
                T3AfterSil1 = results.second;
            }
        }

        // 7->
        // we are ready to reconstruct Eex with all resolutions implemented
        //(d,light) is investigated gating on Esil1 = 0!
        bool cutEAfterSil0 {T3AfterSil0 == 0.};
        bool cutELoss0 {eLoss0Cut.first <= eLoss0 && eLoss0 <= eLoss0Cut.second};
        if(cutEAfterSil0 && cutELoss0) // fill histograms
        {
            auto T3Recon {runner.EnergyBeforeGas(eLoss0, distance0, "light")};
            auto EexAfter {kingen.GetBinaryKinematics().ReconstructExcitationEnergy(T3Recon, theta3Lab)};
            auto theta3CM {TMath::Pi() - kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Recon, theta3Lab)};

            // fill histograms
            hThetaCM->Fill(theta3CM * TMath::RadToDeg());
            hEexBefore->Fill(EexBefore, weight); // with the weight from each TGenPhaseSpace::Generate()
            hDistL0->Fill(distance0);
            hKinVertex->Fill(theta3Lab * TMath::RadToDeg(), T3Recon);
            hEexAfter->Fill(EexAfter, weight);
            hSP->Fill(silPoint0InMM.Y(), silPoint0InMM.Z());
            // Fill histogram of SP with thetaCM as weight
            hSPTheta->Fill(silPoint0InMM.Y(), silPoint0InMM.Z(), theta3CM * TMath::RadToDeg());
            // RP histogram
            hRP->Fill(vertex.X(), vertex.Y());
            // Fill new efficiency histograms
            if(E796Gates::rpx1(vertex))
                hThetaCM1->Fill(theta3CM * TMath::RadToDeg());
            if(E796Gates::rpx2(vertex))
                hThetaCM2->Fill(theta3CM * TMath::RadToDeg());

            // write to TTree
            Eex_tree = EexAfter;
            weight_tree = weight;
            theta3CM_tree = theta3CM * TMath::RadToDeg();
            EVertex_tree = T3Recon;
            theta3Lab_tree = theta3Lab * TMath::RadToDeg();
            rpx_tree = vertex.X();
            hRPxSimu->Fill(vertex.X());
            outTree->Fill();
        }
    }
    std::cout << "\r" << std::string(100 / percentPrint, '|') << 100 << "%";
    std::cout.flush();
    std::cout << RESET << '\n';

    // Efficiencies as quotient of histograms in TEfficiency class
    auto* eff {new TEfficiency(*hThetaCM, *hThetaCMAll)};
    eff->SetNameTitle("eff", TString::Format("#theta_{CM} eff E_{x} = %.2f MeV", Ex));
    // Efficiencies for the other sections of actar
    auto* eff1 {new TEfficiency {*hThetaCM1, *hThetaCMAll1}};
    eff1->SetNameTitle("eff1", TString::Format("#theta_{CM} eff section 1 E_{x} = %.2f MeV", Ex));
    auto* eff2 {new TEfficiency {*hThetaCM2, *hThetaCMAll2}};
    eff2->SetNameTitle("eff2", TString::Format("#theta_{CM} eff section 2 E_{x} = %.2f MeV", Ex));

    // SAVING
    outFile->cd();
    outTree->Write();
    eff->Write();
    eff1->Write();
    eff2->Write();
    outFile->Close();
    delete outFile;
    outFile = nullptr;

    // plotting
    if(standalone)
    {
        auto* c0 {new TCanvas("c0", "Canvas for inspection 0")};
        c0->DivideSquare(4);
        c0->cd(1);
        hThetaCM->DrawClone();
        c0->cd(2);
        hThetaCMAll->DrawClone();
        c0->cd(3);
        hEexBefore->DrawClone("hist");
        c0->cd(4);
        // hThetaCMAll->DrawClone();
        hRP->DrawClone("colz");

        // draw theoretical kinematics
        ActPhysics::Kinematics theokin {p1, p2, p3, p4, T1 * p1.GetAMU(), Ex};
        auto* gtheo {theokin.GetKinematicLine3()};

        auto* c1 {new TCanvas("cAfter", "Canvas for inspection 1")};
        c1->DivideSquare(6);
        c1->cd(1);
        hKinVertex->DrawClone("colz");
        gtheo->Draw("same");
        c1->cd(2);
        hSP->DrawClone("col");
        sm->Draw();
        c1->cd(3);
        hEexAfter->DrawClone("hist");
        c1->cd(4);
        eff->Draw("apl");
        c1->cd(5);
        hSPTheta->DrawClone("colz");
        auto* p6 {c1->cd(6)};
        p6->Divide(2, 1);
        p6->cd(1);
        eff1->Draw("apl");
        p6->cd(2);
        eff2->Draw("apl");

        // hRPx->DrawNormalized();
        // hRPxSimu->SetLineColor(kRed);
        // hRPxSimu->DrawNormalized("same");
    }

    // deleting news
    delete geometry;
    delete srim;
    delete rand;

    timer.Stop();
    timer.Print();
}
