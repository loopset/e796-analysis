#ifndef Analyser_cxx
#define Analyser_cxx

#include "ActColors.h"
#include "ActKinematics.h"
#include "ActSRIM.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TEnv.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom.h"
#include "TString.h"
#include "TTree.h"

#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3D.h"

#include <iostream>
#include <string>

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

LambdaFilter lambdaPunch0 {[](int idx0, double eafter0, int idx1, double eafter1)
                           { return (idx0 != -1) && (eafter0 <= 0); }};

LambdaFilter lambdaPunch1 {[](int idx0, double eafter0, int idx1, double eafter1)
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

std::string ToStandardName(const std::string& name)
{
    if(name == "d")
        return "2H";
    if(name == "t")
        return "3H";
    return name;
}

TString
GetSimuFile(const std::string& beam, const std::string& target, const std::string& light, double ebeam, double ex)
{
    TString iterPath {gEnv->GetValue("IterPath", "./")};
    std::cout << BOLDMAGENTA << "Open simu in :" << iterPath << '\n';
    std::cout << "  for : "
              << TString::Format("%s(%s,%s)@%.2f|%.2f", beam.c_str(), target.c_str(), light.c_str(), ebeam, ex) << RESET
              << '\n';
    return TString::Format("./%s/Outputs/simu_%s_%s_%s_ebeam_%.2f_ex_%.2f.root", iterPath.Data(), beam.c_str(),
                           ToStandardName(target).c_str(), ToStandardName(light).c_str(), ebeam, ex);
}

TString
GetAnaFile(const std::string& beam, const std::string& target, const std::string& light, double ebeam, double ex)
{
    TString iterPath {gEnv->GetValue("IterPath", "./")};
    std::cout << BOLDGREEN << "Open ana in :" << iterPath << '\n';
    std::cout << "  for : "
              << TString::Format("%s(%s,%s)@%.2f|%.2f", beam.c_str(), target.c_str(), light.c_str(), ebeam, ex) << RESET
              << '\n';
    return TString::Format("./%s/Outputs/ana_%s_%s_%s_ebeam_%.2f_ex_%.2f.root", iterPath.Data(), beam.c_str(),
                           target.c_str(), light.c_str(), ebeam, ex);
}


RetAna Analyse(const std::string& beam, const std::string& target, const std::string& light, double ebeam, double ex,
               bool draw = false, const LambdaFilter& lambdaFilter = lambdaPunch0)
{
    // Is elastic?
    bool isEl {target == light};

    // Angular uncertainty arising from reconstruction
    // Estimated from 20O(d,d) analysis, where its effects should be more prominent
    // and then validated using 20O(d,t)
    double sigmaTheraRec {0.25}; // deg

    ROOT::EnableImplicitMT();
    auto infile {GetSimuFile(beam, target, light, ebeam, ex)};
    ROOT::RDataFrame df {"ActGeant", infile};

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
    // Geant4 table
    TString iterPath {gEnv->GetValue("IterPath", "./")};
    srim.ReadTable("light",
                   TString::Format("./%s/Outputs/dedx/table_%s_GasMixture.txt", iterPath.Data(), aux.c_str()).Data(),
                   false);

    // Kinematics
    ActPhysics::Kinematics kin {
        TString::Format("%s(%s,%s)@%.2f|%.2f", beam.c_str(), target.c_str(), light.c_str(), ebeam, ex).Data()};
    std::vector<ActPhysics::Kinematics> vkins {df.GetNSlots()};
    for(auto& k : vkins)
        k = kin;

    // Gate on events
    auto gated {df.Filter(lambdaFilter, {"SilIdx0", "SilEAfter0", "SilIdx1", "SilEAfter1"})
                    .Filter(
                        [&](ROOT::RVecC& layer0)
                        {
                            // Fucking GEANT4 writes string as a vector and on top of that
                            // adds a fucking empty space at the end...............
                            std::string aux {layer0.begin(), layer0.begin() + 2};
                            if(isEl) // do not count front layers for elastic reactions
                                return (aux == "l0") || (aux == "r0");
                            else // ignore zd silicons for transfer
                            {
                                // Index of layer
                                auto it {aux.find_first_of("0123456789")};
                                int idx {-1};
                                if(it != std::string::npos)
                                    idx = std::stoi(aux.substr(it));
                                return (idx == 0) || (idx == 1);
                            }
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
                          [&](ROOT::RVecD& window, ROOT::RVecD& vertex, ROOT::RVecD& sil0)
                          {
                              ROOT::Math::XYZPoint wp {window[0], window[1], window[2]};
                              ROOT::Math::XYZPointD rp {vertex[0], vertex[1], vertex[2]};
                              ROOT::Math::XYZPointD sp {sil0[0], sil0[1], sil0[2]};
                              auto beamDir {rp - wp};
                              auto lightDir {sp - rp};
                              auto dot {lightDir.Unit().Dot(beamDir.Unit())};
                              auto theta {TMath::ACos(dot) * TMath::RadToDeg()};
                              // Add reconstruction impact on theta
                              theta = gRandom->Gaus(theta, sigmaTheraRec);
                              return theta;
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
    // hEx->GetFunction("gaus")->ResetBit(TF1::kNotDraw);

    // Compute efficiency
    auto* eff {new TEfficiency {*hCMAfter, *hCMAll}};
    eff->SetTitle(TString::Format("%.2f", ex));

    // Define return value
    RetAna ret {.hKinSampled = (TH2D*)hKinSampled->Clone(),
                .hKin = (TH2D*)hKin->Clone(),
                .hPID = (TH2D*)hPID->Clone(),
                .hCMAll = (TH1D*)hCMAll->Clone(),
                .hCMAfter = (TH1D*)hCMAfter->Clone(),
                .hEx = (TH1D*)hEx->Clone(),
                .eff = eff};

    // Write
    auto anafile {GetAnaFile(beam, target, light, ebeam, ex)};
    def.Snapshot("AnaTree", anafile);
    auto fout {std::make_unique<TFile>(anafile, "update")};
    ret.hKinSampled->Write("hKinSampled");
    ret.hKin->Write("hKin");
    ret.hPID->Write("hPID");
    ret.hCMAll->Write("hCMAll");
    ret.hCMAfter->Write("hCMAfter");
    ret.hEx->Write("hEx");
    ret.eff->Write("eff");

    if(draw)
    {
        static int cAnaIdx {0};
        auto* c {new TCanvas {TString::Format("c%d", cAnaIdx), TString::Format("Simu canvas Ex = %.2f", ex)}};
        cAnaIdx++;
        c->DivideSquare(6);
        c->cd(1);
        ret.hKinSampled->Draw("colz");
        c->cd(2);
        ret.hKin->Draw("colz");
        ActPhysics::Kinematics k {
            TString::Format("%s(%s,%s)@%.2f|%.2f", beam.c_str(), target.c_str(), light.c_str(), ebeam, ex).Data()};
        k.GetKinematicLine3()->Draw("l");
        c->cd(3);
        ret.hPID->Draw("colz");
        c->cd(4);
        ret.hEx->Draw();
        c->cd(5);
        ret.eff->Draw("apl");
        c->cd(6);
        // Divide by sin (thetaCM)
        auto* fsolid {new TF1 {"fsolid", "TMath::Sin(x * TMath::DegToRad())", 0, 180}};
        ret.hCMAll->Divide(fsolid);
        ret.hCMAll->Draw();
    }

    return ret;
}

#endif