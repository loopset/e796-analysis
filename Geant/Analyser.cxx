#ifndef Analyser_cxx
#define Analyser_cxx

#include "ActKinematics.h"
#include "ActSRIM.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TStyle.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"

class RetAna
{
public:
    TH2D* hKinSampled {};
    TH2D* hKin {};
    TH2D* hPID {};
    TH1D* hCMAll {};
    TH1D* hCMAfter {};
    TH1D* hEx {};
    TEfficiency* eff {};
};

using LambdaFilter = std::function<bool(int, double, int, double)>;

LambdaFilter lambdPunch0 {[](int idx0, double eafter0, int idx1, double eafter1)
                          { return (idx0 != -1) && (eafter0 <= 0); }};

LambdaFilter lambdPunch1 {[](int idx0, double eafter0, int idx1, double eafter1)
                          {
                              if(idx0 != -1)
                              {
                                  if(eafter0 > 0)
                                  {
                                      if(eafter1 <= 0)
                                          return idx0 == idx1;
                                      return false;
                                  }
                                  return true;
                              }
                              return false;
                          }};


TString
GetSimuFile(const std::string& beam, const std::string& target, const std::string& light, double ebeam, double ex)
{
    return TString::Format("./Outputs/simu_%s_%s_%s_ebeam_%.2f_ex_%.2f.root", beam.c_str(), target.c_str(),
                           light.c_str(), ebeam, ex);
}

RetAna analyse(const std::string& beam, const std::string& target, const std::string& light, double ebeam, double ex,
               const std::string& others = "", const LambdaFilter& lambdaFilter = lambdPunch0)
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"ActGeant", GetSimuFile(beam, target, light, ebeam, ex)};
    // Book histogram with all thetaCM
    auto hCMAll {df.Histo1D(HistConfig::ThetaCM, "thetaCM")};

    std::string aux {};
    if(light == "d" || light == "2H")
        aux = "deuteron";
    else if(light == "t" || light == "3H")
        aux = "triton";
    else if(light == "3He")
        aux = "He3";
    else if(light == "4He")
        aux = "alpha";
    else
        aux = light;

    // SRIM
    ActPhysics::SRIM srim;
    srim.ReadTable("light", TString::Format("./Outputs/dedx/table_%s_GasMixture.txt", aux.c_str()).Data());

    // Kinematics
    ActPhysics::Kinematics kin {"20O(d,d)@700|0"};
    std::vector<ActPhysics::Kinematics> vkins {df.GetNSlots()};
    for(auto& k : vkins)
        k = kin;

    // Gate on events
    auto gated {df.Filter(lambdaFilter, {"SilIdx0", "SilEAfter0", "SilIdx1", "SilEAfter1"})};
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

    // Define return value
    RetAna ret {.hKinSampled = (TH2D*)hKin->Clone(),
                .hKin = (TH2D*)hKinRec->Clone(),
                .hPID = (TH2D*)hPID->Clone(),
                .hCMAll = (TH1D*)hCMAll->Clone(),
                .hCMAfter = (TH1D*)hCMAfter->Clone(),
                .hEx = (TH1D*)hEx->Clone(),
                .eff = eff};
    return ret;
}

#endif