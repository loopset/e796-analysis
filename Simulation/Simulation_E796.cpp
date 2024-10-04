#include "ActColors.h"
#include "ActCutsManager.h"
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

void ApplySilRes(double& e, double sigma)
{
    e = gRandom->Gaus(e, sigma * TMath::Sqrt(e / 5.5));
}

double AngleWithNormal(const ROOT::Math::XYZVector& dir, const ROOT::Math::XYZVector& normal)
{
    auto dot {dir.Unit().Dot(normal.Unit())};
    return TMath::ACos(dot);
}

void Simulation_E796(const std::string& beam, const std::string& target, const std::string& light, int neutronPS,
                     int protonPS, double T1, double Ex, bool standalone)
{
    // set batch mode if not an independent function
    if(!standalone)
        gROOT->SetBatch(true);

    // Set whether is elastic or not
    const bool isEl {target == light};
    // Set assembly indexes as in the BuilGeometry file
    const int idxAssembly0 {(isEl) ? 2 : 0};
    const int idxAssembly1 {(isEl) ? -1 : 1};

    // SIGMAS
    const double sigmaSil {0.060 / 2.355};
    const double sigmaPercentBeam {0.008};
    const double sigmaAngleLight {0.95 / 2.355};
    // Parameters of beam in mm
    // Beam has to be manually placed in the simulation
    // Centered in Z and Y with a width of 4 mm
    // Center in Z
    const double zOffsetBeam {(isEl) ? 6.97 : 8.34}; // mm
    const double zVertexMean {128. + zOffsetBeam};
    const double zVertexSigma {3.8};
    // Center in Y
    const double yVertexMean {126.5}; // according to juan's emittance
    const double yVertexSigma {2.1};
    // const double zVertexMean {83.59};
    // const double zVertexSigma {3.79};

    // THRESHOLDS FOR SILICONS
    const double thresholdSi0 {1.};
    const double thresholdSi1 {1.};

    // number of iterations
    const int iterations {static_cast<int>((isEl) ? 5e7 : 3e7)};

    // ACTIVATE STRAGGLING OR NOT
    bool stragglingInGas {true};
    bool stragglingInSil {true};
    bool silResolution {true};
    bool thetaResolution {true};

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

    // Silicon masks
    auto* sm {E796Utils::GetEffSilMatrix(target, light)};
    if(isEl)
    {
        sm->MoveZTo(zVertexMean - zOffsetBeam, {3, 4, 5});
    }
    else
        sm->MoveZTo(zVertexMean - zOffsetBeam, {3, 4});

    // RPx histogram to sample
    // auto* hRPx {GetRPXProj(E796Utils::GetFileName(2, beam, target, light, false, "rpx"))};
    // // // histogram file
    // Histogram to sample vertex
    auto* histfile {new TFile("./Inputs/BeamY_ThetaXY_and_XZ_space_3Dhisto.root")};
    auto* hBeam {histfile->Get<TH3F>("h_Y_thetaXY_thetaXZ")};
    if(!hBeam)
        throw std::runtime_error("Simulation_E796(): Could not load beam emittance histogram");
    // closing hBeam file and coming back to main ROOT directory
    gROOT->cd();
    hBeam->SetDirectory(nullptr);
    histfile->Close();
    delete histfile;
    histfile = nullptr;

    //---- SIMULATION STARTS HERE
    ROOT::EnableImplicitMT();

    // timer
    TStopwatch timer {};
    timer.Start();

    // Init particles
    ActPhysics::Particle p1 {beam};
    ActPhysics::Particle p2 {target};
    ActPhysics::Particle p3 {light};
    // Automatically compute 4th particle
    ActPhysics::Kinematics tkin {beam, target, light};
    ActPhysics::Particle p4 {tkin.GetParticle(4)};
    // Init kinematics generator
    ActSim::KinematicGenerator kingen {p1, p2, p3, p4, protonPS, neutronPS};
    kingen.Print();

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

    auto hRPz {std::make_unique<TH2D>("hRPz", "SP;Y [mm];Z [mm]", 550, 0, 256, 550, 0, 256)};

    auto* hRPxSimu {HistConfig::RP.GetHistogram()->ProjectionX("hRPxSimu")};

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

    // Load geometry
    auto* geometry {new ActSim::Geometry()};
    geometry->ReadGeometry("/media/Data/E796v2/Simulation/Geometries/", "geo_all");

    // Random generator
    auto* rand {new TRandom3()};
    rand->SetSeed(); // random path in each execution of macro

    // Runner: contains utility functions to execute multiple actions
    ActSim::Runner runner(srim, geometry, rand, sigmaSil);

    // Output from simulation!
    // We only store a few things in the TTree
    // 1-> Excitation energy
    // 2-> Theta in CM frame
    // 3-> Weight of the generator: for three-body reactions (phase spaces) the other two
    // variables need to be weighted by this value. For binary reactions, weight = 1
    // 4-> Energy at vertex
    // 5-> Theta in Lab frame
    auto* outFile {new TFile(gSelector->GetSimuFile(beam, target, light, Ex, neutronPS, protonPS), "recreate")};
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
        // 1-> Sample vertex
        auto [start, vertex] {runner.SampleVertex(yVertexMean, yVertexSigma, zVertexMean, zVertexSigma, hBeam)};

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
        // obtain thetas and energies
        auto* PLight {kingen.GetLorentzVector(0)};
        auto theta3Lab {PLight->Theta()};
        auto phi3Lab {PLight->Phi()};
        auto T3Lab {PLight->Energy() - p3.GetMass()};
        // to compute geometric efficiency by CM interval and with our set reference direction
        double theta3CMBefore {kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Lab, theta3Lab)};
        hThetaCMAll->Fill(theta3CMBefore * TMath::RadToDeg());

        // 4-> Include thetaLab resolution to compute thetaCM and Ex afterwards
        if(thetaResolution) // resolution in
            theta3Lab = runner.GetRand()->Gaus(theta3Lab, sigmaAngleLight * TMath::DegToRad());

        // 4.1 -> Apply cut on vertex position
        if(!E796Gates::rp(vertex.X()))
            continue;

        // 5-> Propagate track from vertex to silicon wall using Geometry class
        // And using the angle with the uncertainty already in
        ROOT::Math::XYZVector dirBeamFrame {TMath::Cos(theta3Lab), TMath::Sin(theta3Lab) * TMath::Sin(phi3Lab),
                                            TMath::Sin(theta3Lab) * TMath::Cos(phi3Lab)};
        // Declare beam direction
        auto beamDir {(vertex - start).Unit()};
        // Rotate to world = geometry frame
        auto dirWorldFrame {runner.RotateToWorldFrame(dirBeamFrame, beamDir)};

        auto vertexInGeoFrame {runner.DisplacePointToTGeometryFrame(vertex)};
        ROOT::Math::XYZPoint silPoint0 {};
        int silType0 {};
        int silIndex0 {};
        double distance0 {};
        bool side0 {};
        // Assembly 0
        runner.GetGeo()->PropagateTrackToSiliconArray(vertexInGeoFrame, dirWorldFrame, idxAssembly0, side0, distance0,
                                                      silType0, silIndex0, silPoint0);
        // convert to mm (Geometry::PropagateTracksToSiliconArray works in cm but we need mm to use in SRIM)
        distance0 *= 10.;

        // skip tracks that doesn't reach silicons or are not in SiliconMatrix
        if(silIndex0 == -1 || !(sm->IsInMatrix(silIndex0)))
            continue;
        auto silPoint0InMM {runner.DisplacePointToStandardFrame(silPoint0)};
        // Apply SilMatrix cut
        if(!sm->IsInside(silIndex0, (isEl ? silPoint0InMM.X() : silPoint0InMM.Y()), silPoint0InMM.Z()))
            continue;
        // And if elastic, apply cut in silicon index
        if(isEl)
            if(!E796Gates::maskelsil(silIndex0))
                continue;

        auto T3EnteringSil {srim->SlowWithStraggling("light", T3Lab, distance0)};
        ApplyNaN(T3EnteringSil);
        // nan if stopped in gas
        if(!std::isfinite(T3EnteringSil))
            continue;

        // First layer of silicons
        // Angle with normal
        auto angleNormal0 {AngleWithNormal(dirWorldFrame, {(isEl ? 0. : 1.), (isEl ? 1. : 0.), 0})};
        auto T3AfterSil0 {srim->SlowWithStraggling("lightInSil", T3EnteringSil,
                                                   geometry->GetAssemblyUnitWidth(idxAssembly0) * 10, angleNormal0)};
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
        int silType1 {};
        bool side1 {};
        ROOT::Math::XYZPoint silPoint1 {};
        double eLoss1 {};
        double T3AfterSil1 {};
        bool isPunch {};
        if(T3AfterSil0 > 0 && !isEl)
        {
            // first, propagate in gas
            runner.GetGeo()->PropagateTrackToSiliconArray(vertexInGeoFrame, dirWorldFrame, 1, side1, distance1,
                                                          silType1, silIndex1, silPoint1, false);
            if(silIndex1 == -1)
                continue;

            distance1 = (silPoint1 - silPoint0).R() * 10;
            T3AfterInterGas = srim->SlowWithStraggling("light", T3AfterSil0, distance1);
            ApplyNaN(T3AfterInterGas);
            if(!std::isfinite(T3AfterInterGas))
                continue;

            // now, silicon if we have energy left
            if(T3AfterInterGas > 0)
            {
                // For e796 angleNormal0 = angleNormal1 but this is not general
                auto angleNormal1 {angleNormal0};
                auto T3AfterSil1 {srim->SlowWithStraggling(
                    "lightInSil", T3AfterInterGas, geometry->GetAssemblyUnitWidth(idxAssembly1) * 10, angleNormal1)};
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
            // auto T3Recon {runner.EnergyBeforeGas(EBefSil0, distance0, "light")};
            auto T3Recon {srim->EvalInitialEnergy("light", EBefSil0, distance0)};
            auto EexAfter {kingen.GetBinaryKinematics().ReconstructExcitationEnergy(T3Recon, theta3Lab)};
            auto theta3CM {kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Recon, theta3Lab)};

            // fill histograms
            hThetaCM->Fill(theta3CM * TMath::RadToDeg());
            hDistF0->Fill(distance0);
            hKinVertex->Fill(theta3Lab * TMath::RadToDeg(), T3Recon);
            hEexAfter->Fill(EexAfter, weight);
            hSP->Fill((isEl) ? silPoint0InMM.X() : silPoint0InMM.Y(), silPoint0InMM.Z());
            // Fill histogram of SP with thetaCM as weight
            hSPTheta->Fill(silPoint0InMM.Y(), silPoint0InMM.Z(), theta3CM * TMath::RadToDeg());
            // RP histogram
            hRP->Fill(vertex.X(), vertex.Y());
            hRPz->Fill(vertex.Y(), vertex.Z());
            hELoss0->Fill(T3EnteringSil, eLoss0);

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

    // SAVING
    outFile->cd();
    outTree->Write();
    eff->Write();
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
    delete geometry;
    delete srim;
    delete rand;

    timer.Stop();
    timer.Print();
}
