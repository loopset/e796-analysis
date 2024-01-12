#include "ActColors.h"
#include "ActCutsManager.h"
#include "ActGeometry.h"
#include "ActKinematicGenerator.h"
#include "ActKinematics.h"
#include "ActParticle.h"
#include "ActRunner.h"
#include "ActSRIM.h"

#include "Rtypes.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"
#include "TROOT.h"
#include "TRandom3.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TTree.h"
#include "TVirtualPad.h"
#include "TEfficiency.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

void Simulation_TRIUMF(const std::string& beam, const std::string& target, const std::string& light,
                       const std::string& heavy, int neutronPS, int protonPS, double T1, double Ex, bool standalone)
{
    // set batch mode if not an independent function
    if(!standalone)
        gROOT->SetBatch();

    // Set number of Si layers
    // Named after BuildGeoTRIUMF when constructing the SiliconAssebly variables
    // must be [0, max) in ActRoot::Geometry
    // Must iterate in this range to get in which layer we have an impact
    const int maxLayers {4};

    // SIGMAS
    // Resolutions to be implemented as gaussians
    const double sigmaSil {0.060 / 2.355};       // Si resolution
    const double sigmaPercentBeam {0.008};       // Energy res of beam (Energy = Energy +- Energy * sigmaPerccentBeam)
    const double sigmaAngleLight {0.95 / 2.355}; // Sigma in angle (obtained by Juan during experimental analysis)

    // Parameters of beam in mm
    // Beam has to be manually placed in the simulation
    // Centered in Z and Y with a width of 4 mm
    // Center in Z
    const double zVertexMean {128.};
    const double zVertexSigma {4};
    // Center in Y
    const double yVertexMean {128.};
    const double yVertexSigma {4};

    // THRESHOLDS FOR SILICONS
    // Minimum energy deposit to be detected in Sil
    const double thresholdSi0 {1.};
    const double thresholdSi1 {1.};

    // NAME OF OUTPUT FILE
    TString fileName {
        TString::Format("./Outputs/transfer_TRIUMF_Eex_%.3f_nPS_%d_pPS_%d.root", Ex, neutronPS, protonPS)};

    // number of iterations
    const int iterations {static_cast<int>(1e6)};

    // ACTIVATE STRAGGLING OR NOT
    // Flag to activate / deactivate different parameters of simu
    bool stragglingInGas {true};
    bool stragglingInSil {true};
    bool silResolution {true};
    bool thetaResolution {true};

    //---- SIMULATION STARTS HERE
    ROOT::EnableImplicitMT();

    // timer: count how much time to execute this program
    TStopwatch timer {};
    timer.Start();

    // Init particles, passed as strings in func arguments
    ActPhysics::Particle p1 {beam};
    ActPhysics::Particle p2 {target};
    ActPhysics::Particle p3 {light};
    ActPhysics::Particle p4 {heavy};
    // Init kinematics generator: wrapper to TGenPhaseSpace
    // Allows simulation of n-body reactions, but does not allow input of
    // different cross section appart from constant. Delete it!
    // For now on, we will focus only on 2-body (we have to figure out sth for 3-body afterwards)
    ActSim::KinematicGenerator kingen {p1, p2, p3, p4, protonPS, neutronPS};
    kingen.Print();
    // get threshold energy
    // Beam losses energy in gas, but since this reaction has a Tbeam threshold. Compute it here since it is constant
    auto beamThreshold {ActPhysics::Kinematics(p1, p2, p3, p4, -1, Ex).GetT1Thresh()};

    // Histograms
    // WARNING! To compute a fine-grain efficiency, we require at least a binning width of 0.25 degrees!
    auto* hThetaCM {new TH1F("hThetaCM", "ThetaCM;#theta_{CM} [degree]", 720, 0., 180.)};
    auto* hThetaCMAll {(TH1F*)hThetaCM->Clone("hThetaCMAll")};
    hThetaCMAll->SetTitle("All thetaCM");
    auto* hThetaLabDebug {(TH1F*)hThetaCM->Clone("hThetaLabDebug")};
    hThetaLabDebug->SetTitle("Theta discriminated in layer 0;#theta_{Lab} [degree]");
    auto* hDistL0 {new TH1F("hDistL0", "Distance vertex to L0;dist [mm]", 300, 0., 600.)};
    auto* hThetaESil {new TH2F("hThetaELab", "Theta vs Energy in Sil0;#theta_{LAB} [degree];E_{Sil0} [MeV]", 140, 0.,
                               180., 100, 0., 60.)};
    auto* hThetaEVertex {(TH2F*)hThetaESil->Clone("hThetaEVertex")};
    hThetaEVertex->SetTitle("Theta vs EVertex; #theta [degree];E_{Vertex} [MeV]");
    auto* hKin {(TH2F*)hThetaESil->Clone("hKin")};
    hKin->SetTitle("Debug LAB kin;#theta_{Lab} [deg];E");
    auto* hSilPoint {new TH2F("hSilPoint", "Silicon point;X or Y [mm];Z [mm]", 100, -10., 290., 100, -10., 290.)};
    std::vector<TH2F*> hsSP {};
    for(int i = 0; i < maxLayers; i++)
    {
        hsSP.push_back((TH2F*)hSilPoint->Clone(TString::Format("hSP%d", i)));
        hsSP.back()->SetTitle(TString::Format("Sil assembly %d", i));
    }
    auto* hEexAfter {new TH1F("hEex", "Eex with all resolutions", 1000, -10., 40.)};
    auto* hEexBefore {(TH1F*)hEexAfter->Clone("hEexBefore")};
    hEexBefore->SetTitle("Eex without any res.");
    auto* hPhiAll {new TH1F("hPhiAll", "Phi in LAB;#phi [deg]", 360, 0, 180)};
    auto* hPhi {(TH1F*)hPhiAll->Clone("hPhi")};

    // Load SRIM tables
    // The name of the file sets particle + medium
    auto* srim {new ActPhysics::SRIM()};
    srim->ReadInterpolations("light", "./../Calibrations/SRIMData/transformed/protons_deuterium_900mb.dat");
    srim->ReadInterpolations("beam", "./../Calibrations/SRIMData/transformed/11Li_deuterium_900mb.dat");
    srim->ReadInterpolations("lightInSil", "./../Calibrations/SRIMData/transformed/protons_silicon.dat");

    // Load geometry
    // Assure having executed BuildGeoTRIUMF macro before
    auto* geometry {new ActSim::Geometry()};
    geometry->ReadGeometry("./Geometries/", "triumf");

    // Random generator
    auto* rand {new TRandom3()};
    rand->SetSeed(); // random path in each execution of macro

    // Runner: contains utility functions to execute multiple actions
    ActSim::Runner runner(srim, geometry, rand, sigmaSil);
    // get SRIM, Geo, TRandom using runner.GetXXXXX
    // using built-in GetEnergyAfterSilicons automatically implements Sil resolutions passed as sigmaSil

    // Output from simulation!
    // We only store a few things in the TTree
    // 1-> Excitation energy
    // 2-> Theta in CM frame
    // 3-> Weight of the generator: for three-body reactions (phase spaces) the other two
    // variables need to be weighted by this value. For binary reactions, weight = 1
    // 4-> Energy at vertex
    // 5-> Theta in Lab frame
    auto* outFile {new TFile(fileName, "recreate")};
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

    // RUN!
    // print fancy info (dont pay attention to this)
    std::cout << BOLDMAGENTA << "Running for Ex = " << Ex << " MeV" << RESET << '\n';
    std::cout << BOLDGREEN;
    const int percentPrint {5};
    int step {iterations / (100 / percentPrint)};
    int nextPrint {step};
    int percent {};
    for(int reaction = 0; reaction < iterations; reaction++)
    {
        // Print progress
        if(reaction >= nextPrint)
        {
            percent = 100 * reaction / iterations;
            std::cout << "\r" << std::string(percent / percentPrint, '|') << percent << "%";
            std::cout.flush();
            nextPrint += step;
        }
        // 1-> Sample vertex: just taking a random point in X c [0, 256 ]mm; y and z obtained by the previously defined
        // parameters
        auto vertex {runner.SampleVertex(yVertexMean, yVertexSigma, zVertexMean, zVertexSigma, nullptr)};
        // std::cout<<"Vertex = "<<vertex<<" mm"<<'\n';
        // 2-> Beam energy according to its sigma
        auto TBeam {runner.RandomizeBeamEnergy(
            T1 * p1.GetAMU(),
            sigmaPercentBeam * T1 * p1.GetAMU())}; // T1 in Mev / u * mass of beam in u = total kinetic energy
        // Slow down it according to vertex position
        TBeam = runner.EnergyAfterGas(TBeam, vertex.X(), "beam");
        // runner energy functions return std::nan when the particle is stopped in the gas!
        // if nan (aka stopped in gas, continue)
        // if not stopped but beam energy below kinematic threshold, continue
        if(std::isnan(TBeam) || TBeam < beamThreshold)
            continue;
        // std::cout<<"TBeam = "<<TBeam<<" MeV"<<'\n';
        // 3-> Run kinematics!
        // This is to be replaced by ActSim::Kinematics
        // 3.1-> Construct the object passing particles and Tbeam and Ex
        // 3.2-> Build yourself the thetaCM and phiCM using uniform distrib
        // 3.3-> Set kinematics through SetRecoilKinematics(thetaCM, phiCM, 3, true)
        // in that call to function, 3 is the particle whose angle are being set; the light in this case
        kingen.SetBeamAndExEnergies(TBeam, Ex);
        double weight {kingen.Generate()};
        // focus on recoil 3 (light)
        // obtain thetas and energies
        auto* PLight {kingen.GetLorentzVector(0)};
        // 3.4-> The you have the kinematics in the lab by just calling the getters: GetTheta3Lab(), GetT3Lab(), etc
        auto theta3Lab {PLight->Theta()};
        auto phi3Lab {PLight->Phi()};
        auto T3Lab {PLight->Energy() - p3.GetMass()};
        hKin->Fill(theta3Lab * TMath::RadToDeg(), T3Lab);
        // to compute geometric efficiency by CM interval and with our set reference direction
        double theta3CMBefore {TMath::Pi() - kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Lab, theta3Lab)};
        hThetaCMAll->Fill(theta3CMBefore * TMath::RadToDeg());
        hPhiAll->Fill(phi3Lab * TMath::RadToDeg());
        // 4-> Include thetaLab resolution to compute thetaCM and Ex
        if(thetaResolution) // resolution in
            theta3Lab = runner.GetRand()->Gaus(theta3Lab, sigmaAngleLight * TMath::DegToRad());
        // std::cout<<"Theta3Lab = "<<theta3Lab * TMath::RadToDeg()<<" degree"<<'\n';
        // std::cout<<"Theta3New = "<<psGenerator.GetThetaFromTLorentzVector(PLight) * TMath::RadToDeg()<<'\n';
        auto theta3CM {TMath::Pi() - kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Lab, theta3Lab)};
        // std::cout<<"Theta3CM = "<<theta3CM * TMath::RadToDeg()<<" degree"<<'\n';
        // auto phi3Lab {runner.GetRand()->Uniform(0., 2 * TMath::Pi())};
        auto EexBefore {kingen.GetBinaryKinematics().ReconstructExcitationEnergy(T3Lab, theta3Lab)};

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
        int hitAssembly0 {};
        int assemblyIndex {0};
        for(int i = 0; i < maxLayers; i++) // iterate over the layer to find in which the track impacts (if does)
        {
            runner.GetGeo()->PropagateTrackToSiliconArray(vertexInGeoFrame, direction, i, side0, distance0, silType0,
                                                          silIndex0, silPoint0);
            if(silIndex0 != -1)
            {
                hitAssembly0 = i;
                break;
            }
        }
        // convert to mm (Geometry::PropagateTracksToSiliconArray works in cm but we need mm to use in SRIM)
        distance0 *= 10.;

        // skip tracks that doesn't reach silicons or are in silicon index cut
        if(silIndex0 == -1)
        {
            hThetaLabDebug->Fill(theta3Lab * TMath::RadToDeg());
            continue;
        }
        // Moving from ActSim::Geometry reference frame to ACTAR standard frame: (0, 0) = (padx = 0, pady = 0)
        auto silPoint0InMM {runner.DisplacePointToStandardFrame(silPoint0)};

        // Obtain energy loss from vertex until silicon impact point
        auto T3EnteringSil {runner.EnergyAfterGas(T3Lab, distance0, "light", stragglingInGas)};

        // nan if stopped in gas
        if(!std::isfinite(T3EnteringSil))
            continue;

        // First layer of silicons!
        // This func returns a pair of values: first = energy loss in silicon, second = energy after silicon
        auto [eLoss0, T3AfterSil0] {
            runner.EnergyAfterSilicons(T3EnteringSil, geometry->GetAssemblyUnitWidth(hitAssembly0) * 10., thresholdSi0,
                                       "lightInSil", silResolution, stragglingInSil)};
        // does not consider angle of track in silicons... we will have to modify it to do so

        // nan if bellow threshold (experimental sil detectors have a threshold energy below which particles are not
        // detected)
        if(!std::isfinite(eLoss0))
            continue;
        // 6-> Same but to silicon layer 1: SKIP THIS BECAUSE WE ARE NOT CONSIDERING PUNCHTHROUGH
        // // SILICON1
        // double T3AfterInterGas {};
        // double distance1 {};
        // int silIndex1 {};
        // int silType1 {};
        // bool side1 {};
        // ROOT::Math::XYZPoint silPoint1 {};
        // double eLoss1 {};
        // double T3AfterSil1 {};
        // if(T3AfterSil0 > 0 && hitAssembly0 == 0)
        // {
        //     // first, propagate in gas
        //     assemblyIndex = 1;
        //     runner.GetGeo()->PropagateTrackToSiliconArray(vertexInGeoFrame, direction, assemblyIndex, side1,
        //     distance1,
        //                                                   silType1, silIndex1, silPoint1, false);
        //     if(silIndex1 == -1)
        //         continue;
        //
        //     distance1 *= 10.;
        //     distance1 -= distance0; // distanceIntergas = distance1 - distance0
        //     T3AfterInterGas = runner.EnergyAfterGas(T3AfterSil0, distance1, "light", stragglingInGas);
        //     if(!std::isfinite(T3AfterInterGas))
        //         continue;
        //
        //     // now, silicon if we have energy left
        //     if(T3AfterInterGas > 0)
        //     {
        //         auto results {runner.EnergyAfterSilicons(T3AfterInterGas, geometry->GetAssemblyUnitWidth(1) * 10.,
        //                                                  thresholdSi1, "lightInSil", silResolution,
        //                                                  stragglingInSil)};
        //         eLoss1 = results.first;
        //         T3AfterSil1 = results.second;
        //     }
        // }

        // 7->
        // we are ready to reconstruct Eex with all resolutions implemented
        // force delete punchthrough (no energy after first layer in any side)
        bool cutPunchThrough {T3AfterSil0 == 0.};
        if(cutPunchThrough) // fill histograms
        {
            // Here we go back in time! From sil energy after implementing all resolutions to Ex
            auto T3Recon {runner.EnergyBeforeGas(eLoss0, distance0, "light")};
            auto EexAfter {kingen.GetBinaryKinematics().ReconstructExcitationEnergy(T3Recon, theta3Lab)};

            // fill histograms
            hThetaCM->Fill(theta3CM * TMath::RadToDeg());
            hPhi->Fill(phi3Lab * TMath::RadToDeg());
            hEexBefore->Fill(EexBefore, weight); // with the weight for each TGenPhaseSpace::Generate()
            hDistL0->Fill(distance0);
            hThetaESil->Fill(theta3Lab * TMath::RadToDeg(), eLoss0);
            hThetaEVertex->Fill(theta3Lab * TMath::RadToDeg(), T3Recon);
            hEexAfter->Fill(EexAfter, weight);
            // Here we fill the silicon point histograms
            // Note that some of them require to fill .X() or .Y() depending on their orientation
            if(hitAssembly0 == 0 || hitAssembly0 == 1 || hitAssembly0 == 2)
                hsSP[hitAssembly0]->Fill(silPoint0InMM.Y(), silPoint0InMM.Z());
            else
                hsSP[hitAssembly0]->Fill(silPoint0InMM.X(), silPoint0InMM.Z());

            // write to TTree
            Eex_tree = EexAfter;
            weight_tree = weight;
            theta3CM_tree = theta3CM * TMath::RadToDeg();
            EVertex_tree = T3Recon;
            theta3Lab_tree = theta3Lab * TMath::RadToDeg();
            outTree->Fill();
        }
    }
    std::cout << "\r" << std::string(100 / percentPrint, '|') << 100 << "%";
    std::cout.flush();
    std::cout << RESET << '\n';

    // compute geometric efficiency as the division of two histograms: thetaCM after all cuts and before them
    // for now, do now pay attention to this
    std::vector<std::pair<double, double>> geoEff {};
    std::vector<std::pair<double, double>> ugeoEff {};
    for(int bin = 1; bin <= hThetaCMAll->GetNbinsX(); bin++)
    {
        auto x {hThetaCMAll->GetBinCenter(bin)};
        auto y0 {hThetaCMAll->GetBinContent(bin)};
        auto y {hThetaCM->GetBinContent(bin)};
        geoEff.push_back({x, y / y0});
        ugeoEff.push_back({x, TMath::Sqrt(y) / y0});
    }
    // another way:
    auto* eff {new TEfficiency(*hThetaCM, *hThetaCMAll)};

    // plotting
    auto* cBefore {new TCanvas("cBefore", "Before implementing most of the resolutions")};
    cBefore->DivideSquare(4);
    cBefore->cd(1);
    hThetaCMAll->Draw();
    hThetaCM->SetLineColor(kRed);
    hThetaCM->Draw("sames");
    cBefore->cd(2);
    hDistL0->Draw();
    cBefore->cd(3);
    hEexBefore->Draw("hist");
    cBefore->cd(4);
    hPhiAll->Draw();
    hPhi->SetLineColor(kRed);
    hPhi->Draw("sames");

    // draw theoretical kinematics
    ActPhysics::Kinematics theokin {p1, p2, p3, p4, T1 * p1.GetAMU(), Ex};
    auto* gtheo {theokin.GetKinematicLine3()};
    auto* cAfter {new TCanvas("cAfter", "After implementing all")};
    cAfter->DivideSquare(4);
    cAfter->cd(1);
    hThetaESil->Draw("col");
    cAfter->cd(2);
    hThetaEVertex->Draw("col");
    gtheo->Draw("same");
    cAfter->cd(3);
    hEexAfter->Draw("hist");
    cAfter->cd(4);
    hKin->Draw("colz");

    auto* cSP {new TCanvas("cSP", "Silicon points")};
    cSP->DivideSquare(hsSP.size());
    for(int i = 0; i < hsSP.size(); i++)
    {
        cSP->cd(i + 1);
        hsSP[i]->Draw("colz");
    }

    // SAVING
    outFile->cd();
    outTree->Write();
    outFile->WriteObject(&geoEff, "efficiency");
    outFile->WriteObject(&ugeoEff, "uefficiency");
    outFile->Close();
    delete outFile;
    outFile = nullptr;

    // deleting news
    delete geometry;
    delete srim;
    delete rand;
    if(!standalone)
    {
        delete cSP;
        delete cAfter;
        delete cBefore;
        delete hThetaCM;
        delete hThetaCMAll;
        delete hThetaLabDebug;
        delete hDistL0;
        delete hThetaESil;
        delete hThetaEVertex;
        delete hSilPoint;
        delete hEexAfter;
        delete hEexBefore;
        delete hPhi;
        delete hPhiAll;
    }


    timer.Stop();
    timer.Print();
}
