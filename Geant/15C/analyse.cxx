#include "ActKinematics.h"
#include "ActSRIM.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TF1.h"
#include "TMath.h"
#include "TRandom.h"
#include "TStyle.h"

#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3D.h"
#include "Math/Vector3Dfwd.h"

#include <iostream>

#include "../PostAnalysis/HistConfig.h"

void analyse()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"ActGeant", "./Outputs/simu_20O_2H_2H_ebeam_700.00_ex_0.00.root"};
    // Book histogram with all thetaCM
    auto hCMAll {df.Histo1D(HistConfig::ThetaCM, "thetaCM")};

    // SRIM
    ActPhysics::SRIM srim;
    srim.ReadTable("light", "../Calibrations/SRIMData/raw/2H_952mb_mixture.txt");
    // srim.ReadTable("light", "./Outputs/dedx/table_triton_GasMixture.txt", false);

    // Kinematics
    ActPhysics::Kinematics kin {"20O(d,d)@700|0"};
    std::vector<ActPhysics::Kinematics> vkins {df.GetNSlots()};
    for(auto& k : vkins)
        k = kin;

    // Gate on events
    auto gated {df.Filter(
        [](int idx0, double eafter0, int idx1, double eafter1)
        {
            return (idx0 != -1) && (eafter0 <= 0);
            // if(idx0 != -1)
            // {
            //     if(eafter0 > 0)
            //     {
            //         if(eafter1 <= 0)
            //             return idx0 == idx1;
            //         return false;
            //     }
            //     return true;
            // }
            // return false;
        },
        {"SilIdx0", "SilEAfter0", "SilIdx1", "SilEAfter1"})};
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
                          [](ROOT::RVecD& window, ROOT::RVecD& vertex, ROOT::RVecD& sil0)
                          {
                              ROOT::Math::XYZPoint wp {window[0], window[1], window[2]};
                              ROOT::Math::XYZPointD rp {vertex[0], vertex[1], vertex[2]};
                              ROOT::Math::XYZPointD sp {sil0[0], sil0[1], sil0[2]};
                              auto beamDir {rp - wp};
                              auto lightDir {sp - rp};
                              auto dot {lightDir.Unit().Dot(beamDir.Unit())};
                              return TMath::ACos(dot) * TMath::RadToDeg();
                          },
                          {"WP", "TPCIni", "SilIni0"})
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
                                  // std::cout << "=============" << '\n';
                                  // std::cout << "SP0 : " << sp0 << '\n';
                                  // std::cout << "SP1 : " << sp1 << '\n';
                                  // std::cout << "Dinter: " << dInterSil << '\n';
                                  // std::cout << "Delta1 : " << DeltaE1 << '\n';
                                  double recEAfter0 {srim.EvalInitialEnergy("light", DeltaE1, dInterSil)};
                                  EAtSil = recEAfter0 + DeltaE0;
                              }
                              else
                                  EAtSil = DeltaE0;
                              return srim.EvalInitialEnergy("light", EAtSil, tl);
                          },
                          {"SilEAfter0", "SilDeltaE0", "SilIni0", "SilIdx1", "SilDeltaE1", "SilIni1", "TL"})
                  .DefineSlot("Ex",
                              [&vkins](unsigned int slot, double theta, double e, double ebeam)
                              {
                                  auto& k {vkins[slot]};
                                  k.SetBeamEnergy(ebeam);
                                  return k.ReconstructExcitationEnergy(e, theta * TMath::DegToRad());
                              },
                              {"thetaLab", "EVertex", "EBeam"})
                  .Define("Diff", "T3 - EVertex")};

    // Book histograms
    auto hKin {def.Histo2D(HistConfig::KinSimu, "theta3", "T3")};
    auto hKinRec {def.Histo2D(HistConfig::KinSimu, "thetaLab", "EVertex")};
    auto hEx {def.Histo1D(HistConfig::Ex, "Ex")};
    auto hDiff {def.Histo1D("Diff")};
    auto hCMAfter {def.Histo1D(HistConfig::ThetaCM, "thetaCM")};
    ROOT::RDF::TH2DModel mPID {"hPID", "PID;E_{Sil} [MeV];#DeltaE_{gas} [MeV]", 200, 0, 20, 200, 0, 2};
    auto hPID {def.Histo2D(mPID, "SilDeltaE0", "TPCDeltaE")};

    // Fit to a gaussian
    hEx->Fit("gaus", "0QR+");
    // hEx->GetFunction("gaus")->ResetBit(TF1::kNotDraw);

    // Compute efficiency
    auto* eff {new TEfficiency {*hCMAfter, *hCMAll}};

    gStyle->SetOptFit();
    auto* c0 {new TCanvas {"c0", "Analysis of simulation"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hKin->DrawClone("colz");
    kin.GetKinematicLine3()->Draw("l");
    c0->cd(2);
    hKinRec->DrawClone("colz");
    kin.GetKinematicLine3()->Draw("l");
    c0->cd(3);
    hEx->DrawClone();
    c0->cd(4);
    eff->Draw("al");
    c0->cd(5);
    hPID->DrawClone("colz");
}