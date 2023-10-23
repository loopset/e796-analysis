#include "TCanvas.h"
#include "TFile.h"
#include "TMath.h"
#include "TROOT.h"
#include "TH2F.h"
#include "TH1F.h"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "TStopwatch.h"
#include "TTree.h"
#include "TRandom3.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "ActColors.h"
#include "ActParticle.h"
#include "ActCutsManager.h"
#include "ActSRIM.h"
#include "ActGeometry.h"
#include "ActKinematics.h"
#include "ActKinematicGenerator.h"
#include "ActRunner.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>
#include <set>

void Simulation_d4He_E796(const std::string& beam, const std::string& target,
                          const std::string& light, const std::string& heavy,
                          int neutronPS, int protonPS,
                          double T1,
                          double Ex,
                          bool standalone)
{
    //set batch mode if not an independent function
    if(!standalone)
        gROOT->SetBatch();
        
    //SIGMAS
    const double sigmaSil { 0.060 / 2.355};
    const double sigmaPercentBeam { 0.008};
    const double sigmaAngleLight { 0.95 / 2.355};
    const double zVertexMean {83.59};
    const double zVertexSigma {3.79};

    //THRESHOLDS FOR SILICONS
    const double thresholdSi0 { 1.};
    const double thresholdSi1 { 1.};

    //NAME OF OUTPUT FILE
    TString fileName {TString::Format("./Outputs/transfer_d4He_Eex_%.3f_nPS_%d_pPS_%d.root", Ex, neutronPS, protonPS)};

    //number of iterations
    const int iterations {static_cast<int>(1e7)};

    //ACTIVATE STRAGGLING OR NOT
    bool stragglingInGas {true};
    bool stragglingInSil {true};
    bool silResolution {true};
    bool thetaResolution {true};

    //CUTS ON SILICON ENERGY
    std::pair<double, double> eLoss0Cut {6.5, 27.};

    //CUTS ON SILICON INDEX
    std::set<int> silIndexCut {1, 6, 7, 9};
    const double zOffsetGraphCuts { - 50.};//lower silPoint to graphical cuts, in mm

    //histogram file
    auto* histfile {new TFile("./Inputs/BeamY_ThetaXY_and_XZ_space_3Dhisto.root")};
    auto* hBeam {histfile->Get<TH3F>("h_Y_thetaXY_thetaXZ")};
    if(!hBeam)
        throw std::runtime_error("Could not load beam emittance histogram");
    //closing hBeam file and coming back to main ROOT directory
    gROOT->cd();
    hBeam->SetDirectory(nullptr);
    histfile->Close();
    delete histfile; histfile = nullptr;

    //---- SIMULATION STARTS HERE
    ROOT::EnableImplicitMT();

    //timer
    TStopwatch timer {}; timer.Start();

    //Init particles
    ActPhysics::Particle p1 {beam}; ActPhysics::Particle p2 {target};
    ActPhysics::Particle p3 {light}; ActPhysics::Particle p4 {heavy};
    //Init kinematics generator
    ActSim::KinematicGenerator kingen {p1, p2, p3, p4, protonPS, neutronPS};
    kingen.Print();
    
    //Read visual silicon cuts: based on silicon position
    ActRoot::CutsManager<std::string> silGraphCuts {};
    for(int i = 0; i < 11; i++)
        silGraphCuts.ReadCut(std::to_string(i), TString::Format("/media/Data/E796v2/Cuts/veto_sil%d.root", i).Data());

    //Histograms
    //WARNING! To compute a fine-grain efficiency, we require at least a binning width of 0.25 degrees!
    auto* hThetaCM {new TH1F("hThetaCM", "ThetaCM;#theta_{CM} [degree]", 720, 0., 180.)};
    auto* hThetaCMAll {(TH1F*) hThetaCM->Clone("hThetaCMAll")}; hThetaCMAll->SetTitle("All thetaCM");
    auto* hThetaLabDebug {(TH1F*)hThetaCM->Clone("hThetaLabDebug")}; hThetaLabDebug->SetTitle("Theta discriminated in layer 0;#theta_{Lab} [degree]");
    auto* hDistL0 {new TH1F("hDistL0", "Distance vertex to L0;dist [mm]", 300, 0., 600.)};
    auto* hThetaESil {new TH2F("hThetaELab", "Theta vs Energy in Sil0;#theta_{LAB} [degree];E_{Sil0} [MeV]", 70, 0., 90., 100, 0., 60.)};
    auto* hThetaEVertex {(TH2F*)hThetaESil->Clone("hThetaEVertex")}; hThetaEVertex->SetTitle("Theta vs EVertex; #theta [degree];E_{Vertex} [MeV]");
    auto* hSilPoint {new TH2F("hSilPoint", "Silicon point & graph cuts;Y [mm];Z [mm]", 100, -10., 290., 100, -10., 290. )};
    auto* hEexAfter {new TH1F("hEex", "Eex with all resolutions", 1000, -10., 40.)};
    auto* hEexBefore {(TH1F*)hEexAfter->Clone("hEexBefore")}; hEexBefore->SetTitle("Eex without any res.");

    //Load SRIM tables
    //The name of the file sets particle + medium
    auto* srim {new ActPhysics::SRIM()};
    srim->ReadInterpolations("4He", "/media/Data/E796v2/Calibrations/SRIMData/transformed/4He_in_952mb_mixture.dat");
    srim->ReadInterpolations("20O", "/media/Data/E796v2/Calibrations/SRIMData/transformed/20O_in_952mb_mixture.dat");
    srim->ReadInterpolations("4HeInSil", "/media/Data/E796v2/Calibrations/SRIMData/transformed/4He_in_silicon.dat");

    //Load geometry
    auto* geometry { new ActSim::Geometry()};
    geometry->ReadGeometry("./Geometries/", "geo0");

    //Random generator
    auto* rand {new TRandom3()}; rand->SetSeed();//random path in each execution of macro

    //Runner: contains utility funcstions to execute multiple actions
    ActSim::Runner runner(srim, geometry, rand, sigmaSil);

    //Output from simulation!
    //We only store a few things in the TTree
    //1-> Excitation energy
    //2-> Theta in CM frame
    //3-> Weight of the generator: for three-body reactions (phase spaces) the other two
    //variables need to be weighted by this value. For binary reactions, weight = 1
    //4-> Energy at vertex
    //5-> Theta in Lab frame
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
    
    //RUN!
    //print fancy info
    std::cout<<BOLDMAGENTA<<"Running for Ex = "<<Ex<<" MeV"<<RESET<<'\n';
    std::cout<<BOLDGREEN;
    const int percentPrint {5};
    int step {iterations / (100 / percentPrint)};
    int nextPrint {step};
    int percent {};
    for(int reaction = 0; reaction < iterations; reaction++)
    {
        //Print progress
        if(reaction >= nextPrint)
        {
            percent = 100 * reaction / iterations;
            std::cout<<"\r"<<std::string(percent / percentPrint, '|')<<percent<<"%";
            std::cout.flush();
            nextPrint += step;
        }
        //1-> Sample vertex
        auto vertex {runner.SampleVertex(-1, -1, zVertexMean, zVertexSigma, hBeam)};
        //std::cout<<"Vertex = "<<vertex<<" mm"<<'\n';
        //2-> Beam energy according to its sigma
        auto TBeam {runner.RandomizeBeamEnergy( T1 * p1.GetAMU() , sigmaPercentBeam * T1 * p1.GetAMU())};//T1 in Mev / u * mass of beam in u = total kinetic energy
        //std::cout<<"TBeam = "<<TBeam<<" MeV"<<'\n';
        //3-> Run kinematics!
        kingen.SetBeamAndExEnergies(TBeam, Ex);
        double weight {kingen.Generate()};
        //focus on recoil 3 (light)
        //obtain thetas and energies
        auto* PLight {kingen.GetLorentzVector(0)};
        auto theta3Lab {PLight->Theta()};
        auto T3Lab {PLight->Energy() - p3.GetMass()};
        //to compute geometric efficiency by CM interval and with our set reference direction
        double theta3CMBefore { TMath::Pi() - kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Lab, theta3Lab)};
        hThetaCMAll->Fill(theta3CMBefore * TMath::RadToDeg());
        //4-> Include thetaLab resolution to compute thetaCM and Ex
        if(thetaResolution)//resolution in 
            theta3Lab = runner.GetRand()->Gaus(theta3Lab, sigmaAngleLight * TMath::DegToRad());
        //std::cout<<"Theta3Lab = "<<theta3Lab * TMath::RadToDeg()<<" degree"<<'\n';
        //std::cout<<"Theta3New = "<<psGenerator.GetThetaFromTLorentzVector(PLight) * TMath::RadToDeg()<<'\n';
        auto theta3CM { TMath::Pi() - kingen.GetBinaryKinematics().ReconstructTheta3CMFromLab(T3Lab, theta3Lab)};
        //std::cout<<"Theta3CM = "<<theta3CM * TMath::RadToDeg()<<" degree"<<'\n';
        auto phi3Lab {runner.GetRand()->Uniform(0., 2 * TMath::Pi())};
        auto EexBefore {kingen.GetBinaryKinematics().ReconstructExcitationEnergy(T3Lab, theta3Lab)};

        //apply cut in XVertex
        if(!(40. <= vertex.X() && vertex.X() <= 200.))
            continue;
        //5-> Propagate track from vertex to silicon wall using Geometry class
        ROOT::Math::XYZVector direction
            {
                TMath::Cos(theta3Lab),
                TMath::Sin(theta3Lab) * TMath::Sin(phi3Lab),
                TMath::Sin(theta3Lab) * TMath::Cos(phi3Lab)
            };
        auto vertexInGeoFrame {runner.DisplacePointToTGeometryFrame(vertex)};
        ROOT::Math::XYZPoint silPoint0 {}; int silType0 {}; int silIndex0 {}; double distance0 {}; bool side0 {};
        //Assembly 0
        int assemblyIndex {0};
        runner.GetGeo()->PropagateTrackToSiliconArray(vertexInGeoFrame, direction, assemblyIndex,
                                                      side0, distance0, silType0, silIndex0, silPoint0);
        //convert to mm (Geometry::PropagateTracksToSiliconArray works in cm but we need mm to use in SRIM)
        distance0 *= 10.;

        //skip tracks that doesn't reach silicons or are in silicon index cut
        if(silIndex0 == -1 || (silIndexCut.find(silIndex0) != silIndexCut.end()))
        {
            hThetaLabDebug->Fill(theta3Lab * TMath::RadToDeg());
            continue;
        }
        //cut with TCutG!
        auto silPoint0InMM {runner.DisplacePointToStandardFrame(silPoint0)};
        silPoint0InMM.SetZ(silPoint0InMM.Z() + zOffsetGraphCuts);
        if(!silGraphCuts.IsInside(std::to_string(silIndex0), silPoint0InMM.Y(), silPoint0InMM.Z()))
            continue;

        auto T3EnteringSil {runner.EnergyAfterGas(T3Lab, distance0, "4He", stragglingInGas)};
        //nan if stopped in gas
        if(!std::isfinite(T3EnteringSil))
            continue;

        //SILICON0
        auto [eLoss0, T3AfterSil0] {runner.EnergyAfterSilicons(T3EnteringSil, geometry->GetAssemblyUnitWidth(0) * 10., thresholdSi0, "4HeInSil", silResolution, stragglingInSil)};
        //nan if bellow threshold
        if(!std::isfinite(eLoss0))
            continue;
        //6-> Same but to silicon layer 1
        //SILICON1
        double T3AfterInterGas {};
        double distance1 {}; int silIndex1 {}; int silType1 {}; bool side1 {};
        ROOT::Math::XYZPoint silPoint1 {};
        double eLoss1 {}; double T3AfterSil1 {};
        if(T3AfterSil0 > 0)
        {
            //first, propagate in gas
            assemblyIndex = 1;
            runner.GetGeo()->PropagateTrackToSiliconArray(vertexInGeoFrame, direction, assemblyIndex,
                                                          side1, distance1, silType1, silIndex1, silPoint1, false);
            if(silIndex1 == -1)
                continue;
            
            distance1 *= 10.;
            T3AfterInterGas = runner.EnergyAfterGas(T3AfterSil0, distance1, "4He", stragglingInGas);

            //now, silicon if we have energy left
            if(T3AfterInterGas > 0)
            {
                auto results {runner.EnergyAfterSilicons(T3AfterInterGas, geometry->GetAssemblyUnitWidth(1) * 10., thresholdSi1, "4HeInSil", silResolution, stragglingInSil)};
                eLoss1      = results.first;
                T3AfterSil1 = results.second;
            }
        }

        //7-> 
        //we are ready to reconstruct Eex with all resolutions implemented
        //(d,4He) is investigated gating on Esil1 = 0!
        bool cutEAfterSil0 {T3AfterSil0 == 0.};
        bool cutELoss0 {eLoss0Cut.first <= eLoss0 && eLoss0 <= eLoss0Cut.second};
        if(cutEAfterSil0 && cutELoss0)//fill histograms
        {
            auto T3Recon {runner.EnergyBeforeGas(eLoss0, distance0, "4He")};
            auto EexAfter {kingen.GetBinaryKinematics().ReconstructExcitationEnergy(T3Recon, theta3Lab)};

            //fill histograms
            hThetaCM->Fill(theta3CM * TMath::RadToDeg());
            hEexBefore->Fill(EexBefore, weight);//with the weight for each TGenPhaseSpace::Generate()
            hDistL0->Fill(distance0);
            hThetaESil->Fill(theta3Lab * TMath::RadToDeg(), eLoss0);
            hThetaEVertex->Fill(theta3Lab * TMath::RadToDeg(), T3Recon);
            hEexAfter->Fill(EexAfter, weight);
            hSilPoint->Fill(silPoint0InMM.Y(), silPoint0InMM.Z());

            //write to TTree
            Eex_tree      = EexAfter;
            weight_tree   = weight;
            theta3CM_tree = theta3CM * TMath::RadToDeg();
            EVertex_tree  = T3Recon;
            theta3Lab_tree = theta3Lab * TMath::RadToDeg();
            outTree->Fill();
        }
    }
    std::cout <<"\r"<< std::string(100/percentPrint , '|') << 100 << "%";
    std::cout.flush(); std::cout<<RESET<<'\n';
    
    //compute geometric efficiency as the division of two histograms: thetaCM after all cuts and before them
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

    //plotting
    auto* cBefore {new TCanvas("cBefore", "Before implementing most of the resolutions")};
    cBefore->DivideSquare(4);
    cBefore->cd(1);
    hThetaCM->Draw();
    cBefore->cd(2);
    hDistL0->Draw();
    cBefore->cd(3);
    hEexBefore->Draw();
    cBefore->cd(4);
    hThetaCMAll->Draw();

    //draw theoretical kinematics
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
    hSilPoint->Draw("col");
    silGraphCuts.DrawAll();

    //SAVING
    outFile->cd();
    outTree->Write();
    outFile->WriteObject(&geoEff, "efficiency");
    outFile->WriteObject(&ugeoEff, "uefficiency");
    outFile->Close();
    delete outFile; outFile = nullptr;

    //deleting news
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
