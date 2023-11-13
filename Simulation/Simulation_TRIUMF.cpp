#include "ActColors.h"
#include "ActCutsManager.h"
#include "ActGeometry.h"
#include "ActKinematicGenerator.h"
#include "ActKinematics.h"
#include "ActParticle.h"
#include "ActRunner.h"
#include "ActSRIM.h"

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
    // must be [0, max) in ActRoot::Geometry
    const int maxLayers {4};

    // SIGMAS
    const double sigmaSil {0.060 / 2.355};
    const double sigmaPercentBeam {0.008};
    const double sigmaAngleLight {0.95 / 2.355};

    // Parameters of beam in mm
    //Center in Z
    const double zVertexMean {128.};
    const double zVertexSigma {4};
    // Center in Y
    const double yVertexMean {128.};
    const double yVertexSigma {4};

    // THRESHOLDS FOR SILICONS
    const double thresholdSi0 {1.};
    const double thresholdSi1 {1.};

    // NAME OF OUTPUT FILE
    TString fileName {
        TString::Format("./Outputs/transfer_TRIUMF_Eex_%.3f_nPS_%d_pPS_%d.root", Ex, neutronPS, protonPS)};

    // number of iterations
    const int iterations {static_cast<int>(1e6)};

    // ACTIVATE STRAGGLING OR NOT
    bool stragglingInGas {true};
    bool stragglingInSil {true};
    bool silResolution {true};
    bool thetaResolution {true};

    // CUTS ON SILICON ENERGY: disabled, allow enter any range of eLoss in layer 0
    std::pair<double, double> eLoss0Cut {0, 1000};

    // CUTS ON SILICON INDEX
    std::set<int> silIndexCut {};
    const double zOffsetGraphCuts {-50.}; // lower silPoint to graphical cuts, in mm

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
    // get threshold energy
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

    // Load SRIM tables
    // The name of the file sets particle + medium
    auto* srim {new ActPhysics::SRIM()};
    srim->ReadInterpolations("light", "./../Calibrations/SRIMData/transformed/proton_in_deuterium_900mb.dat");
    srim->ReadInterpolations("beam", "./../Calibrations/SRIMData/transformed/11Li_in_deuterium_900mb.dat");
    srim->ReadInterpolations("lightInSil", "./../Calibrations/SRIMData/transformed/proton_in_silicon.dat");

    // Load geometry
    auto* geometry {new ActSim::Geometry()};
    geometry->ReadGeometry("./Geometries/", "triumf");

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
    // print fancy info
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
        // 1-> Sample vertex
        auto vertex {runner.SampleVertex(yVertexMean, yVertexSigma, zVertexMean, zVertexSigma, nullptr)};
        // std::cout<<"Vertex = "<<vertex<<" mm"<<'\n';
        // 2-> Beam energy according to its sigma
        auto TBeam {runner.RandomizeBeamEnergy(
            T1 * p1.GetAMU(), 
            sigmaPercentBeam * T1 * p1.GetAMU())}; // T1 in Mev / u * mass of beam in u = total kinetic energy
        // Slow down it according to vertex position
        TBeam = runner.EnergyAfterGas(TBeam, vertex.X(), "beam");
        // if nan (aka stopped in gas, continue) 
        // if not stopped but beam energy below kinematic threshold, continue
        if(std::isnan(TBeam) || TBeam < beamThreshold)
            continue;
        // std::cout<<"TBeam = "<<TBeam<<" MeV"<<'\n';
        // 3-> Run kinematics!
        kingen.SetBeamAndExEnergies(TBeam, Ex);
        double weight {kingen.Generate()};
        // focus on recoil 3 (light)
        // obtain thetas and energies
        auto* PLight {kingen.GetLorentzVector(0)};
        auto theta3Lab {PLight->Theta()};
        auto T3Lab {PLight->Energy() - p3.GetMass()};
        hKin->Fill(theta3Lab * TMath::RadToDeg(), T3Lab);
        // to compute geometric efficiency by CM interval and with our set reference direction
        double theta3CMBefore {TMath::Pi() - kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Lab, theta3Lab)};
        hThetaCMAll->Fill(theta3CMBefore * TMath::RadToDeg());
        // 4-> Include thetaLab resolution to compute thetaCM and Ex
        if(thetaResolution) // resolution in
            theta3Lab = runner.GetRand()->Gaus(theta3Lab, sigmaAngleLight * TMath::DegToRad());
        // std::cout<<"Theta3Lab = "<<theta3Lab * TMath::RadToDeg()<<" degree"<<'\n';
        // std::cout<<"Theta3New = "<<psGenerator.GetThetaFromTLorentzVector(PLight) * TMath::RadToDeg()<<'\n';
        auto theta3CM {TMath::Pi() - kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Lab, theta3Lab)};
        // std::cout<<"Theta3CM = "<<theta3CM * TMath::RadToDeg()<<" degree"<<'\n';
        auto phi3Lab {runner.GetRand()->Uniform(0., 2 * TMath::Pi())};
        auto EexBefore {kingen.GetBinaryKinematics().ReconstructExcitationEnergy(T3Lab, theta3Lab)};

        // apply cut in XVertex
        // if(!(40. <= vertex.X() && vertex.X() <= 200.))
        //     continue;
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
        for(int i = 0; i < maxLayers; i++)
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
        if(silIndex0 == -1 || (silIndexCut.find(silIndex0) != silIndexCut.end()))
        {
            hThetaLabDebug->Fill(theta3Lab * TMath::RadToDeg());
            continue;
        }
        // cut with TCutG!
        auto silPoint0InMM {runner.DisplacePointToStandardFrame(silPoint0)};

        auto T3EnteringSil {runner.EnergyAfterGas(T3Lab, distance0, "light", stragglingInGas)};
        // nan if stopped in gas
        if(!std::isfinite(T3EnteringSil))
            continue;

        // SILICON0
        auto [eLoss0,
              T3AfterSil0] {runner.EnergyAfterSilicons(T3EnteringSil, geometry->GetAssemblyUnitWidth(hitAssembly0) * 10.,
                                                       thresholdSi0, "lightInSil", silResolution, stragglingInSil)};
        if(hitAssembly0 == 2 && std::isfinite(eLoss0))
        {
            std::cout << "1H reached backwards silicons!!!" << '\n';
        }
        // nan if bellow threshold
        if(!std::isfinite(eLoss0))
            continue;
        // // 6-> Same but to silicon layer 1
        // // SILICON1
        // double T3AfterInterGas {};
        // double distance1 {};
        // int silIndex1 {};
        // int silType1 {};
        // bool side1 {};
        // ROOT::Math::XYZPoint silPoint1 {};
        // double eLoss1 {};
        // double T3AfterSil1 {};
        // if(T3AfterSil0 > 0)
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
        //     T3AfterInterGas = runner.EnergyAfterGas(T3AfterSil0, distance1, "light", stragglingInGas);
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
        bool cutEAfterSil0 {T3AfterSil0 == 0.};
        bool cutELoss0 {eLoss0Cut.first <= eLoss0 && eLoss0 <= eLoss0Cut.second};
        if(cutEAfterSil0 && cutELoss0) // fill histograms
        {
            auto T3Recon {runner.EnergyBeforeGas(eLoss0, distance0, "light")};
            auto EexAfter {kingen.GetBinaryKinematics().ReconstructExcitationEnergy(T3Recon, theta3Lab)};

            // fill histograms
            hThetaCM->Fill(theta3CM * TMath::RadToDeg());
            hEexBefore->Fill(EexBefore, weight); // with the weight for each TGenPhaseSpace::Generate()
            hDistL0->Fill(distance0);
            hThetaESil->Fill(theta3Lab * TMath::RadToDeg(), eLoss0);
            hThetaEVertex->Fill(theta3Lab * TMath::RadToDeg(), T3Recon);
            hEexAfter->Fill(EexAfter, weight);
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

    // plotting
    auto* cBefore {new TCanvas("cBefore", "Before implementing most of the resolutions")};
    cBefore->DivideSquare(4);
    cBefore->cd(1);
    hThetaCM->Draw();
    cBefore->cd(2);
    hDistL0->Draw();
    cBefore->cd(3);
    hEexBefore->Draw("hist");
    cBefore->cd(4);
    hThetaCMAll->Draw();

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
    }


    timer.Stop();
    timer.Print();
}
