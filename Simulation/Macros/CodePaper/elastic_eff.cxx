#include "ActCutsManager.h"
#include "ActKinematicGenerator.h"
#include "ActKinematics.h"
#include "ActParticle.h"
#include "ActRunner.h"
#include "ActSRIM.h"
#include "ActSilSpecs.h"
#include "ActTPCParameters.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TH2.h"
#include "TH3.h"
#include "TMath.h"
#include "TRandom.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TTree.h"

#include <iostream>
#include <string>

#include "../../../PostAnalysis/Gates.cxx"
#include "../../../PostAnalysis/Utils.cxx"

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

void elastic_eff()
{
    std::string target {"2H"};
    std::string light {"2H"};
    bool isEl {target == light};
    // Beam energy
    double T1 {35}; // AMeV
    // Ex
    double Ex {0};

    // Iterations
    const int iterations {static_cast<int>(1e6)};

    // TPC basic parameters
    ActRoot::TPCParameters tpc {"Actar"};

    // Uncertainties
    const double sigmaSil {0.060 / 2.355};
    const double sigmaAngleLight {0.95 / 2.355};

    // Vertex sampling
    double zVertexSigma {3.5};
    auto beamfile {std::make_unique<TFile>("/media/Data/E796v2/Macros/Emittance/Outputs/histos.root")};
    auto* hBeam {beamfile->Get<TH3D>("h3d")};
    if(!hBeam)
        throw std::runtime_error("Simulation_E796(): Could not load beam emittance histogram");
    hBeam->SetDirectory(nullptr);
    beamfile.reset();

    // Kinematics
    ActPhysics::Particle p1 {"20O"};
    ActPhysics::Particle p2 {target};
    ActPhysics::Particle p3 {light};
    // Automatically compute 4th particle
    ActPhysics::Kinematics kaux {p1, p2, p3};
    ActPhysics::Particle p4 {kaux.GetParticle(4)};
    // Binary kinematics generator
    ActSim::KinematicGenerator kingen {p1, p2, p3, p4, 0, 0};
    kingen.Print();


    // Silicon specs
    auto* specs {new ActPhysics::SilSpecs};
    specs->ReadFile("../../../configs/detailedSilicons.conf");
    // Silicon EFFECTIVE matrix
    ActPhysics::SilMatrix* sm {};
    // Set reference position and offset along Z!
    double silCentre {};
    double beamOffset {}; // determined from emittance calculations
    // Set layers
    std::string firstLayer {};
    std::string secondLayer {};
    if(isEl)
    {
        sm = E796Utils::GetSideMatrix();
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
        sm = E796Utils::GetFrontSilMatrix(light);
        silCentre = sm->GetMeanZ({3, 4});
        beamOffset = 9.01; // mm
        specs->GetLayer("f0").ReplaceWithMatrix(sm);
        specs->GetLayer("f1").MoveZTo(silCentre, {3, 4});
        specs->EraseLayer("l0");
        firstLayer = "f0";
        secondLayer = "f1";
    }
    double zVertexMean {silCentre + beamOffset};

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

    // Load SRIM tables
    // The name of the file sets particle + medium
    auto* srim {new ActPhysics::SRIM()};
    srim->ReadTable(
        "light",
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", light.c_str()).Data());
    srim->ReadTable("beam",
                    TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", "20O").Data());
    srim->ReadTable(
        "lightInSil",
        TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_silicon.txt", light.c_str()).Data());

    // Random generator
    gRandom->SetSeed();
    // Runner: contains utility functions to execute multiple actions
    ActSim::Runner runner(nullptr, nullptr, gRandom, 0);

    // histograms
    auto* hThetaDelta {new TH2D {"hThetaDelta", ";#theta_{Lab} [#circ];#DeltaE [MeV]", 200, 40, 100, 200, 0, 15}};

    // Saving to file
    auto* file {new TFile {TString::Format("./Outputs/punch_%s_%s.root", target.c_str(), light.c_str()), "recreate"}};
    auto* tree {new TTree {"MiniTree", "A simplified simulation of punchthrough"}};
    double deltae_tree {};
    tree->Branch("DeltaE", &deltae_tree);
    double theta3lab_tree {};
    tree->Branch("ThetaLight", &theta3lab_tree);

    // timer
    TStopwatch timer {};
    timer.Start();
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
            T1 * p1.GetAMU(), 0.008 * T1 * p1.GetAMU())}; // T1 in Mev / u * mass of beam in u = total kinetic energy
        // And slow according to distance travelled
        auto distToVertex {(vertex - start).R()};
        TBeam = srim->Slow("beam", TBeam, distToVertex);

        // 3-> Run kinematics!
        kingen.SetBeamAndExEnergies(TBeam, Ex);
        // Uniform phi always and it is the same for CM and Lab
        auto phiCM {gRandom->Uniform(0, TMath::TwoPi())};
        // thetaCM following xs or not
        double thetaCM {thetaCM = TMath::ACos(gRandom->Uniform(-1, 1))};
        // Ptr to current binary kinematics
        auto* simkin {kingen.GetBinaryKinematics()};
        simkin->ComputeRecoilKinematics(thetaCM, phiCM);
        auto theta3Lab {simkin->GetTheta3Lab()};
        theta3Lab = gRandom->Gaus(theta3Lab, sigmaAngleLight * TMath::DegToRad());
        auto phi3Lab {simkin->GetPhi3Lab()};
        auto T3Lab {simkin->GetT3Lab()};

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
        // Apply cuts in sil index
        if(isEl)
        {
            if(!E796Gates::maskelsil(silIndex0))
                continue;
        }
        else
        {
            if(!E796Gates::masktranssil(silIndex0))
                continue;
        }

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
        ApplyNaN(eLoss0, 0.5, "thresh");
        // nan if bellow threshold
        if(!std::isfinite(eLoss0))
            continue;

        // Fill
        hThetaDelta->Fill(theta3Lab * TMath::RadToDeg(), eLoss0);
        deltae_tree = eLoss0;
        theta3lab_tree = theta3Lab;
        tree->Fill();
    }

    std::cout << "\r" << std::string(100 / percentPrint, '|') << 100 << "%";
    std::cout.flush();
    std::cout << RESET << '\n';
    timer.Stop();
    timer.Print();

    // Write
    file->Write();
    file->Close();

    // Draw
    auto* c0 {new TCanvas {"c0", "Elastic eff"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hThetaDelta->Draw("colz");
}
