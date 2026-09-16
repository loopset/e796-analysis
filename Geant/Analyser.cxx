#ifndef Analyser_cxx
#define Analyser_cxx

#include "ActColors.h"
#include "ActKinematics.h"
#include "ActSRIM.h"

#include "ROOT/RDF/HistoModels.hxx"
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
    TH2D* hEStragg {};
};

bool IsL1(int idx0, ROOT::RVecD& tpcEnd)
{
    double validateWidth {7 * 2}; // 7 pads
    if(idx0 == -1)
    {
        auto x {(-128 + validateWidth) <= tpcEnd[0] && tpcEnd[0] <= (128 - validateWidth)};
        auto y {(-128 + validateWidth) <= tpcEnd[1] && tpcEnd[1] <= (128 - validateWidth)};
        auto z {(-128 + validateWidth) <= tpcEnd[2] && tpcEnd[2] <= (128 - validateWidth)};
        return x && y && z;
    }
    else
        return false;
}

using LambdaFilter = std::function<bool(int, double, int, double, ROOT::RVecD& tpcEnd)>;

LambdaFilter lambdaPunch0 {[](int idx0, double eafter0, int idx1, double eafter1, ROOT::RVecD& tpcEnd)
                           {
                               // Test if L1
                               if(idx0 == -1)
                               {
                                   return IsL1(idx0, tpcEnd);
                               }
                               else // Si trigger
                                   return (idx0 != -1) && (eafter0 <= 0);
                           }};

LambdaFilter lambdaPunch1 {[](int idx0, double eafter0, int idx1, double eafter1, ROOT::RVecD& tpcEnd)
                           {
                               // Test if L1
                               if(idx0 == -1)
                               {
                                   IsL1(idx0, tpcEnd);
                               }
                               else // Si trigger
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
    if(name == "p")
        return "1H";
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
    // Estimated from S2384 results
    double sigmaThetaRecSi {};
    double sigmaThetaRecL1 {};
    if(isEl)
    {
        sigmaThetaRecSi = 0.6;
        sigmaThetaRecL1 = 1.5;
    }
    else
    {
        sigmaThetaRecSi = 0.6;
        sigmaThetaRecL1 = 2.6;
    }

    ROOT::EnableImplicitMT();
    auto infile {GetSimuFile(beam, target, light, ebeam, ex)};
    ROOT::RDataFrame df {"ActGeant", infile};

    std::string aux {};
    if(light == "d" || light == "2H")
        aux = "deuteron";
    else if(light == "1H" || light == "p")
        aux = "proton";
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
    auto gated {
        df.Filter(lambdaFilter, {"SilIdx0", "SilEAfter0", "SilIdx1", "SilEAfter1", "TPCEnd"})
            .Define("IsL1", [](int idx0, ROOT::RVecD& tpcEnd) { return IsL1(idx0, tpcEnd); }, {"SilIdx0", "TPCEnd"})
            .Filter(
                [&](bool isl1, ROOT::RVecD& tpcEnd, ROOT::RVecC& layer0)
                {
                    // Define exlusion zone of L1 trigger
                    if(isl1)
                    {
                        auto yEnd {tpcEnd[1]};
                        double ycenter {0};
                        double exclusionWidth {15 * 2}; // 16 mm up and down center
                        return TMath::Abs(yEnd) > (ycenter + exclusionWidth);
                    }
                    else // gate on Si layer for Si triggers
                    {
                        // Fucking GEANT4 writes string as a vector and on top of that
                        // adds a fucking empty space at the end...............
                        std::string aux {layer0.begin(), layer0.begin() + 2};
                        if(isEl) // do not count front layers for elastic reactions
                            return (aux == "l0") || (aux == "r0");
                        else // ignore zd silicons for transfer
                        {
                            // Index of FRONT layer
                            auto it {aux.find_first_of("0123456789")};
                            int idx {-1};
                            if(it != std::string::npos)
                                idx = std::stoi(aux.substr(it));
                            return (idx == 0) || (idx == 1);
                        }
                    }
                },
                {"IsL1", "TPCEnd", "SilLayer0"})};
    // Define variables
    auto def {
        gated
            .Define("TL",
                    [](bool isl1, ROOT::RVecD& tpcIni, ROOT::RVecD& tpcEnd, ROOT::RVecD& sil0)
                    {
                        ROOT::Math::XYZPointD ini {tpcIni[0], tpcIni[1], tpcIni[2]};
                        ROOT::Math::XYZPointD end {};
                        if(isl1)
                            end = {tpcEnd[0], tpcEnd[1], tpcEnd[2]};
                        else
                            end = {sil0[0], sil0[1], sil0[2]};
                        return (ini - end).R();
                    },
                    {"IsL1", "TPCIni", "TPCEnd", "SilIni0"})
            .Define("thetaLab",
                    [&](bool isl1, ROOT::RVecD& window, ROOT::RVecD& vertex, ROOT::RVecD& tpcEnd, ROOT::RVecD& sil0)
                    {
                        ROOT::Math::XYZPoint wp {window[0], window[1], window[2]};
                        ROOT::Math::XYZPointD rp {vertex[0], vertex[1], vertex[2]};
                        ROOT::Math::XYZPoint end {};
                        if(isl1)
                            end = {tpcEnd[0], tpcEnd[1], tpcEnd[2]};
                        else
                            end = {sil0[0], sil0[1], sil0[2]};
                        auto beamDir {rp - wp};
                        auto lightDir {end - rp};
                        auto dot {lightDir.Unit().Dot(beamDir.Unit())};
                        auto theta {TMath::ACos(dot) * TMath::RadToDeg()};
                        // Add reconstruction impact on theta
                        if(isl1)
                            theta = gRandom->Gaus(theta, sigmaThetaRecL1);
                        else
                            theta = gRandom->Gaus(theta, sigmaThetaRecSi);
                        return theta;
                    },
                    {"IsL1", "WP", "TPCIni", "TPCEnd", "SilIni0"})
            .Define(
                "EVertex",
                [&](bool isl1, double DeltaETPC, double EAfter0, double DeltaE0, ROOT::RVecD& sil0, int idx1,
                    double DeltaE1, ROOT::RVecD& sil1, double tl)
                {
                    if(isl1)
                        return srim.EvalEnergy("light", tl);
                    else
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
                    }
                },
                {"IsL1", "TPCDeltaE", "SilEAfter0", "SilDeltaE0", "SilIni0", "SilIdx1", "SilDeltaE1", "SilIni1", "TL"})
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
            .Define("EStragg", [&](double t3, double evertex) { return evertex - t3; }, {"T3", "EVertex"})
            .Define("Diff", "T3 - EVertex")};

    // Book histograms
    // ThetaCM all goes WITH ALL STATS
    auto hCMAll {df.Histo1D(HistConfig::ThetaCM, "thetaCM")};
    auto hKinSampled {def.Histo2D(HistConfig::KinEl, "theta3", "T3")};
    hKinSampled->SetName("hKinSampled");
    auto hKin {def.Histo2D(HistConfig::KinEl, "thetaLab", "EVertex")};
    auto hEx {def.Histo1D(HistConfig::Ex, "Ex")};
    auto hDiff {def.Histo1D("Diff")};
    auto hCMAfter {def.Histo1D(HistConfig::ThetaCM, "thetaCM")};
    ROOT::RDF::TH2DModel mPID {"hPID", "PID;E_{Sil} [MeV];#DeltaE_{gas} / TL_{drift} [MeV]", 400, 0, 80, 200, 0, 0.1};
    auto hPID {def.Histo2D(mPID, "SilDeltaE0", "Qave")};
    // Fit to a gaussian
    hEx->Fit("gaus", "0Q+");
    // hEx->GetFunction("gaus")->ResetBit(TF1::kNotDraw);
    // Straggling histo
    ROOT::RDF::TH2DModel mEStragg {"hEStragg", "E straggling;T_{3} [MeV];T_{3, rec} [MeV]", 2000, 0, 100, 400, -1, 1};
    auto hEStragg {def.Histo2D(mEStragg, "T3", "EStragg")};
    auto hEBeam {def.Histo1D({"hEBeam", "EBeam", 300, 100, 300}, "EBeam")};

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
                .eff = eff,
                .hEStragg = (TH2D*)hEStragg->Clone()};

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
    ret.hEStragg->Write("hEStragg");
    fout->Close();

    if(draw)
    {
        static int cAnaIdx {0};
        auto* c {new TCanvas {TString::Format("c%d", cAnaIdx), TString::Format("Simu canvas Ex = %.2f", ex)}};
        cAnaIdx++;
        c->DivideSquare(8);
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
        c->cd(7);
        hEBeam->DrawClone();
    }

    return ret;
}

#endif
