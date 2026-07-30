#include "ActColors.h"
#include "ActKinematics.h"
#include "ActSRIM.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TMath.h"
#include "TRandom.h"
#include "TString.h"
#include "TStyle.h"

#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3D.h"

#include <iostream>
#include <string>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
void getAngularStraggling()
{
    // Define channel
    bool isEl {false};
    TString infile {};
    ActPhysics::Kinematics kin {};
    ActPhysics::SRIM srim;
    if(isEl)
    {
        std::cout << BOLDYELLOW << "Elastic channel" << RESET << '\n';
        infile = "./Outputs/simu_20O_2H_2H_ebeam_700.00_ex_0.00.root";
        kin = ActPhysics::Kinematics {"20O(d,d)@700|0"};
        srim.ReadTable("light", "./Outputs/dedx/table_deuteron_GasMixture.txt", false);
    }
    else //(d,t)
    {
        std::cout << BOLDCYAN << "(d,t) channel" << RESET << '\n';
        infile = "./Outputs/simu_20O_2H_3H_ebeam_700.00_ex_0.00.root";
        kin = ActPhysics::Kinematics {"20O(d,t)@700|0"};
        srim.ReadTable("light", "./Outputs/dedx/table_triton_GasMixture.txt", false);
    }


    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"ActGeant", infile};


    // Kinematics
    std::vector<ActPhysics::Kinematics> vkins {df.GetNSlots()};
    for(auto& k : vkins)
        k = kin;

    // Gate on events
    auto gated {df.Filter([](int idx0, double eafter0, int idx1, double eafter1)
                          { return (idx0 != -1) && (eafter0 <= 0); },
                          {"SilIdx0", "SilEAfter0", "SilIdx1", "SilEAfter1"})
                    .Filter(
                        [&](ROOT::RVecC& layer0)
                        {
                            // Fucking GEANT4 writes string as a vector and on top of that
                            // adds a fucking empty space at the end...............
                            std::string aux {layer0.begin(), layer0.begin() + 2};
                            return (aux == "l0") || (aux == "r0");
                        },
                        {"SilLayer0"})};
    // Define variables
    auto def {gated
                  .Define("TL",
                          [](ROOT::RVecD& tpc, ROOT::RVecD& sil0)
                          {
                              ROOT::Math::XYZPointD ini {tpc[0], tpc[1], tpc[2]};
                              ROOT::Math::XYZPointD end {sil0[0], sil0[1], sil0[2]};
                              return (ini - end).R();
                          },
                          {"TPCIni", "SilIni0"})
                  .Define("thetaLab",
                          [](ROOT::RVecD& window, ROOT::RVecD& vertex, ROOT::RVecD& sil0, double thetaSampled)
                          {
                              ROOT::Math::XYZPoint wp {window[0], window[1], window[2]};
                              ROOT::Math::XYZPointD rp {vertex[0], vertex[1], vertex[2]};
                              ROOT::Math::XYZPointD sp {sil0[0], sil0[1], sil0[2]};
                              auto beamDir {rp - wp};
                              auto lightDir {sp - rp};
                              auto dot {lightDir.Unit().Dot(beamDir.Unit())};
                              auto theta {TMath::ACos(dot) * TMath::RadToDeg()};
                              theta = gRandom->Gaus(theta, 0.25); // this is the required sigma arising from
                                                                  // RECONSTRUCTION to match exp 20O g.s. Ex res
                              return theta;
                          },
                          {"WP", "TPCIni", "SilIni0", "theta3"})
                  .Define("EVertex",
                          [&](double EAfter0, double DeltaE0, ROOT::RVecD& sil0, int idx1, double DeltaE1,
                              ROOT::RVecD& sil1, double tl)
                          {
                              double EAtSil {};
                              if(EAfter0 > 0)
                              {
                                  ROOT::Math::XYZPointD sp0 {sil0[0], sil0[1], sil0[2]};
                                  ROOT::Math::XYZPointD sp1 {sil1[0], sil1[1], sil1[2]};
                                  auto dInterSil {(sp0 - sp1).R()};
                                  double recEAfter0 {srim.EvalInitialEnergy("light", DeltaE1, dInterSil)};
                                  EAtSil = recEAfter0 + DeltaE0;
                              }
                              else
                                  EAtSil = DeltaE0;
                              return srim.EvalInitialEnergy("light", EAtSil, tl);
                          },
                          {"SilEAfter0", "SilDeltaE0", "SilIni0", "SilIdx1", "SilDeltaE1", "SilIni1", "TL"})
                  .DefineSlot("Ex",
                              [&vkins](unsigned int slot, double theta, double e, double ebeam, double eSampled)
                              {
                                  auto& k {vkins[slot]};
                                  k.SetBeamEnergy(ebeam);
                                  return k.ReconstructExcitationEnergy(e, theta * TMath::DegToRad());
                              },
                              {"thetaLab", "EVertex", "EBeam", "T3"})
                  .Define("Qave",
                          [](double deltae, ROOT::RVecD& ini, ROOT::RVecD& end)
                          {
                              // TL in drift region
                              ROOT::Math::XYZPoint rp {ini[0], ini[1], ini[2]};
                              ROOT::Math::XYZPoint bp {end[0], end[1], end[2]};
                              auto tl {(rp - bp).R()};
                              return deltae / tl;
                          },
                          {"TPCDeltaE", "TPCIni", "TPCEnd"})
                  .Define("Diff", "T3 - EVertex")};

    // Book histograms
    // ThetaCM all goes WITH ALL STATS
    auto hCMAll {df.Histo1D(HistConfig::ThetaCM, "thetaCM")};
    auto hKinSampled {def.Histo2D(HistConfig::KinSimu, "theta3", "T3")};
    hKinSampled->SetName("hKinSampled");
    auto hKin {def.Histo2D(HistConfig::KinSimu, "thetaLab", "EVertex")};
    auto hEx {def.Histo1D(HistConfig::Ex, "Ex")};
    auto hDiff {def.Histo1D("Diff")};
    auto hCMAfter {def.Histo1D(HistConfig::ThetaCM, "thetaCM")};
    ROOT::RDF::TH2DModel mPID {"hPID", "PID;E_{Sil} [MeV];#DeltaE_{gas} / TL_{drift} [MeV]", 400, 0, 80, 200, 0, 0.1};
    auto hPID {def.Histo2D(mPID, "SilDeltaE0", "Qave")};
    // Fit to a gaussian
    hEx->Fit("gaus", "0QR+");
    auto* f {hEx->GetFunction("gaus")};
    f->ResetBit(TF1::kNotDraw);
    double sigmaGeant {f->GetParameter("Sigma")};
    double sigmaExp {0.300}; // from thesis
    // So the remaining part must be the contribution of RECONSTRUCTION in the angle
    auto sigmaRec {TMath::Sqrt(sigmaExp * sigmaExp - sigmaGeant * sigmaGeant)};
    std::cout << "=============================" << '\n';
    std::cout << "Sigma g.s. from Geant4: " << sigmaGeant << " MeV" << '\n';
    std::cout << "Sigma g.s. from thesis : " << sigmaExp << " MeV" << '\n';
    std::cout << "Sigma g.s. from rec : " << sigmaRec << " MeV" << '\n';


    // Draw
    gStyle->SetOptFit();
    auto* c0 {new TCanvas {"c0", "Angular straggling canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hKin->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();
}
