#include "ActConstants.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TMath.h"
#include "TVirtualPad.h"

#include "Math/Boost.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"

#include <vector>

#include "../../../Selector/Selector.h"

struct CMKin
{
    double fE {};
    double fP {};
    double fTheta {};
    double fPhi {};
};


double ComputeAngle(double thetap, double thetan, double phip, double phin)
{
    // back to radians
    thetap *= TMath::DegToRad();
    thetan *= TMath::DegToRad();
    phip *= TMath::DegToRad();
    phin *= TMath::DegToRad();
    // Build vectors
    ROOT::Math::XYZVector proton {TMath::Cos(thetap), TMath::Sin(thetap) * TMath::Sin(phip),
                                  TMath::Sin(thetap) * TMath::Cos(phip)};
    ROOT::Math::XYZVector neutron {TMath::Cos(thetan), TMath::Sin(thetan) * TMath::Sin(phin),
                                   TMath::Sin(thetan) * TMath::Cos(phin)};
    // Product
    auto dot {proton.unit().Dot(neutron.unit())};
    auto theta {TMath::ACos(dot) * TMath::RadToDeg()};
    return theta;
}

void apply()
{
    gSelector->SetTarget("2H");
    gSelector->SetLight("2H");

    // Read file
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"SimulationTTree", gSelector->GetSimuFile(0, -1, 0)};
    // Define angles
    auto def {df.Define("ep", [](const std::vector<ROOT::Math::XYZTVector>& v)
                        { return v.at(1).E() - ActPhysics::Constants::kpMass; }, {"Lor"})
                  .Define("en", [](const std::vector<ROOT::Math::XYZTVector>& v)
                          { return v.at(2).E() - ActPhysics::Constants::knMass; }, {"Lor"})
                  .Define("thetap", [](const std::vector<ROOT::Math::XYZTVector>& v)
                          { return v.at(1).Theta() * TMath::RadToDeg(); }, {"Lor"})
                  .Define("thetan", [](const std::vector<ROOT::Math::XYZTVector>& v)
                          { return v.at(2).Theta() * TMath::RadToDeg(); }, {"Lor"})
                  .Define("phip", [](const std::vector<ROOT::Math::XYZTVector>& v)
                          { return v.at(1).Phi() * TMath::RadToDeg(); }, {"Lor"})
                  .Define("phin", [](const std::vector<ROOT::Math::XYZTVector>& v)
                          { return v.at(2).Phi() * TMath::RadToDeg(); }, {"Lor"})
                  .Define("CM",
                          [](const std::vector<ROOT::Math::XYZTVector>& v, const ROOT::Math::XYZVector& beta)
                          {
                              // Declare transformation
                              ROOT::Math::Boost boost {-1 * beta}; // probably need to multiply by -1 bc we are using
                                                                   // TLorentzVector beta that is inversed
                              // Transform
                              std::vector<CMKin> cms;
                              for(int i = 1; i <= 2; i++)
                              {
                                  auto lor = v.at(i);
                                  auto cm = boost(lor);
                                  double mass {};
                                  if(i == 1)
                                      mass = ActPhysics::Constants::kpMass;
                                  else
                                      mass = ActPhysics::Constants::knMass;
                                  cms.push_back({.fE = cm.E() - mass,
                                                 .fP = cm.P(),
                                                 .fTheta = 180. - cm.Theta() * TMath::RadToDeg(),
                                                 .fPhi = cm.Phi() * TMath::RadToDeg()});
                              }
                              return cms;
                          },
                          {"Lor", "Beta"})
                  .Define("CMHeavy",
                          [](const std::vector<ROOT::Math::XYZTVector>& v, const ROOT::Math::XYZVector& beta)
                          {
                              // Declare transformation
                              ROOT::Math::Boost boost {-1 * beta}; // probably need to multiply by -1 bc we are using
                                                                   // TLorentzVector beta that is inversed
                              auto lor = v.at(0);
                              auto cm = boost(lor);
                              double mass {18629.589}; // 20O Mass
                              return CMKin {.fE = cm.E() - mass,
                                            .fP = cm.P(),
                                            .fTheta = 180. - cm.Theta() * TMath::RadToDeg(),
                                            .fPhi = cm.Phi() * TMath::RadToDeg()};
                          },
                          {"Lor", "Beta"})
                  .Define("epCM", [](const std::vector<CMKin>& v) { return v.front().fE; }, {"CM"})
                  .Define("enCM", [](const std::vector<CMKin>& v) { return v.back().fE; }, {"CM"})
                  .Define("thetapCM", [](const std::vector<CMKin>& v) { return v.front().fTheta; }, {"CM"})
                  .Define("thetanCM", [](const std::vector<CMKin>& v) { return v.back().fTheta; }, {"CM"})
                  .Define("thetapn", [](double thetap, double thetan, double phip, double phin)
                          { return ComputeAngle(thetap, thetan, phip, phin); }, {"thetap", "thetan", "phip", "phin"})
                  .Define("thetapnCM",
                          [](const std::vector<CMKin>& cms)
                          {
                              // Proton is first
                              auto thetap {cms.front().fTheta};
                              auto phip {cms.front().fPhi};
                              // Neutron
                              auto thetan {cms.back().fTheta};
                              auto phin {cms.back().fPhi};
                              return ComputeAngle(thetap, thetan, phip, phin);
                          },
                          {"CM"})
                  .Define("PpCM", [](const std::vector<CMKin>& v) { return v.front().fP; }, {"CM"})
                  .Define("PnCM", [](const std::vector<CMKin>& v) { return v.back().fP; }, {"CM"})
                  .Define("POCM", [](const CMKin& heavy) { return heavy.fP; }, {"CMHeavy"})
                  .Define("DeltaPCM", [](const std::vector<CMKin>& v) { return std::abs(v.front().fP - v.back().fP); },
                          {"CM"})};
    // Correcting function
    auto* func {new TF1 {"func", "1. - 1. / 180 * x", 0, 180}};
    double cutoff {137.13};
    auto corr {
        [&](double w, double deltap)
        {
            if(deltap > cutoff)
                return 0.;
            else
                return w * (1 - TMath::Power(deltap / cutoff, 1));
        },

    };
    // auto* func {new TF1 {"func", "1. / (x + 1)", 0, 180}};
    def = def.Define("weight_trans", corr, {"weight", "DeltaPCM"});

    // Book histograms
    auto hLabThetaPN {
        def.Histo2D({"hThetaLabPN", "Lab #theta correlations;#theta_{p};#theta_{n}", 300, -180, 180, 300, -180, 180},
                    "thetap", "thetan")};
    auto hLabThetaPNW {def.Histo2D({"hpnw", "#theta_{pn} vs weight;#theta_{pn} [#circ];weight", 300, 0, 180, 800, 0, 1},
                                   "thetapn", "weight")};
    auto hLabEs {def.Histo2D({"hLabEs", "Lab energy correlation;T_{p} [MeV];T_{n} [MeV]", 300, 0, 100, 300, 0, 400},
                             "ep", "en")};
    auto hLabKinP {
        def.Histo2D({"hLabKinP", "Lab p kin;#theta_{Lab, p};E_{p}", 200, 0, 180, 200, 0, 20}, "thetap", "ep", "weight")};
    auto hLabKinPtrans {
        def.Histo2D({"hLabKinPtrans", "Transformed lab p kin;#theta_{Lab, p};E_{p}", 200, 0, 180, 200, 0, 20}, "thetap", "ep", "weight_trans")};


    // CM kinematics
    auto hCMThetaPN {
        def.Histo2D({"hCMThetaPN", "CM #theta correlations;#theta_{p};#theta_{n}", 300, -180, 180, 300, -180, 180},
                    "thetapCM", "thetanCM")};
    auto hCMEs {def.Histo2D({"htwoes", "CM energy correlation;T_{p};T_{n}", 300, 0, 200, 300, 0, 200}, "epCM", "enCM")};
    auto hCMKinP {def.Histo2D({"hKin", "CM p kin;#theta_{CM};E", 200, 0, 180, 200, 0, 200}, "thetapCM", "epCM")};
    auto hCMThetaPNW {
        def.Histo2D({"hpnw", "CM #theta_{pn} vs weight;#theta_{pn} [#circ];weight", 300, 0, 180, 800, 0, 1},
                    "thetapnCM", "weight")};
    auto hpnwtranscm {def.Histo2D(
        {"hpnw", "CM #theta_{pn} vs weight corrected;#theta_{pn} [#circ];weight_trans", 300, 0, 180, 800, 0, 1},
        "thetapnCM", "weight_trans")};
    auto hDeltaP {def.Histo2D({"hDeltaP", "#Delta P in CM;Proton P; Diff (Pp - Pn)", 1000, 0, 600, 400, 0, 200}, "PpCM",
                              "DeltaPCM")};
    auto hDeltaPW {
        def.Histo2D({"hDeltaPW", "DeltaP weight;DeltaP;weight", 400, 0, 200, 200, 0, 1}, "DeltaPCM", "weight")};

    // Before transformation
    auto hExBefore {def.Histo1D({"hEx", "Ex;E_{x} [MeV];Counts", 300, -20, 20}, "Eex", "weight")};
    hExBefore->SetTitle("Before");
    hExBefore->SetLineColor(kRed);
    // After transformation
    auto hExAfter {def.Histo1D({"hEx", "Ex;E_{x} [MeV];Counts", 300, -20, 20}, "Eex", "weight_trans")};
    hExAfter->SetTitle("After");
    hExAfter->SetLineColor(kGreen);
    //
    // // Weight investigation with thetaCM
    // auto hExW {def.Histo2D({"hExW", "Ex vs w;E_{x} [MeV];w", 200, -20, 20, 600, 0, 1}, "Eex", "weight")};

    // Snapshot
    def.Snapshot("SimulationTTree", "./Outputs/d_breakup_trans.root",
                 {"Eex", "theta3CM", "EVertex", "weight", "weight_trans", "DeltaPCM"});

    // Draw
    auto* c0 {new TCanvas {"c0", "LAB kinematics"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hLabThetaPN->DrawClone("colz");
    c0->cd(2);
    hLabThetaPNW->DrawClone("colz");
    c0->cd(3);
    hLabEs->DrawClone("colz");
    c0->cd(4);
    hLabKinP->DrawClone("colz");
    c0->cd(5);
    hLabKinPtrans->DrawClone("colz");
    // hExBefore->SetLineColor(kRed);
    // hExBefore->DrawNormalized("histe");
    // hExAfter->SetLineColor(kGreen);
    // hExAfter->DrawNormalized("histe same");
    // gPad->BuildLegend();
    // c0->cd(5);
    // hLabKinP->DrawClone("colz");
    // c0->cd(6);
    // // hthetas->DrawClone("colz");
    // htwoes->DrawClone("colz");


    // CM canvas
    auto* c1 {new TCanvas {"c1", "CM Canvas"}};
    c1->DivideSquare(8);
    c1->cd(1);
    hCMThetaPN->DrawClone("colz");
    c1->cd(2);
    hCMThetaPNW->DrawClone("colz");
    c1->cd(3);
    hCMEs->DrawClone("colz");
    c1->cd(4);
    hCMKinP->DrawClone("colz");
    c1->cd(5);
    hDeltaP->DrawClone("colz");
    c1->cd(6);
    hDeltaPW->DrawClone("colz");
    c1->cd(7);
    hExBefore->DrawNormalized();
    hExAfter->DrawNormalized("same");
    gPad->BuildLegend();
}
