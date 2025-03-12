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
                                                 .fTheta = 180. - cm.Theta() * TMath::RadToDeg(),
                                                 .fPhi = cm.Phi() * TMath::RadToDeg()});
                              }
                              return cms;
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
                          {"CM"})};
    // Correcting function
    auto* func {new TF1 {"func", "1. - 1. / 180 * x", 0, 180}};
    // auto* func {new TF1 {"func", "1. / (x + 1)", 0, 180}};
    def = def.Define("weight_trans", [&](double w, double thetapn) { return w * func->Eval(thetapn); },
                     {"weight", "thetapnCM"});

    // Book histograms
    auto hpn {def.Histo2D({"hpn", "PN correlation;#theta_{p};#theta_{n}", 300, -180, 180, 300, -180, 180}, "thetap",
                          "thetan")};
    auto hpnw {def.Histo2D({"hpnw", "#theta_{pn} vs weight;#theta_{pn} [#circ];weight", 300, 0, 180, 800, 0, 1},
                           "thetapn", "weight")};
    auto hpnwtrans {def.Histo2D(
        {"hpnw", "#theta_{pn} vs weight corrected;#theta_{pn} [#circ];weight_trans", 300, 0, 180, 800, 0, 1}, "thetapn",
        "weight_trans")};
    auto hEs {def.Histo2D({"hEs", "Energy correlation;T_{p} [MeV];T_{n} [MeV]", 300, 0, 100, 300, 0, 400}, "ep", "en")};
    auto hkin {def.Histo2D({"hKin", "Lab p kin;#theta_{Lab};E", 200, 0, 180, 200, 0, 200}, "thetap", "ep")};
    // CM kinematics
    auto hpncm {def.Histo2D({"hpn", "CM pn correlation;#theta_{p};#theta_{n}", 300, -180, 180, 300, -180, 180},
                            "thetapCM", "thetanCM")};
    auto hpnwcm {def.Histo2D({"hpnw", "CM #theta_{pn} vs weight;#theta_{pn} [#circ];weight", 300, 0, 180, 800, 0, 1},
                             "thetapnCM", "weight")};
    auto hpnwtranscm {def.Histo2D(
        {"hpnw", "CM #theta_{pn} vs weight corrected;#theta_{pn} [#circ];weight_trans", 300, 0, 180, 800, 0, 1},
        "thetapnCM", "weight_trans")};
    auto hkincm {def.Histo2D({"hKin", "CM p kin;#theta_{Lab};E", 200, 0, 180, 200, 0, 200}, "thetapCM", "epCM")};
    auto hthetas {def.Histo2D({"htheta", "Theta p Lab and CM;CM;Lab", 300, 0, 180, 300, 0, 180}, "thetapCM", "thetap")};
    auto htwoes {def.Histo2D({"htwoes", "Ep vs En;E_{p};E_{n}", 300, 0, 200, 300, 0, 200}, "epCM", "enCM")};

    // Before transformation
    auto hExBefore {def.Histo1D({"hEx", "Ex;E_{x} [MeV];Counts", 300, -20, 20}, "Eex", "weight")};
    hExBefore->SetTitle("Before");
    // After transformation
    auto hExAfter {def.Histo1D({"hEx", "Ex;E_{x} [MeV];Counts", 300, -20, 20}, "Eex", "weight_trans")};
    hExAfter->SetTitle("After");

    // Snapshot
    def.Snapshot("SimulationTTree", "./Outputs/d_breakup_trans.root", {"Eex", "EVertex", "weight", "weight_trans"});

    // Draw
    auto* c0 {new TCanvas {"c0", "correlation canvas"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hpn->DrawClone("colz");
    c0->cd(2);
    hpnw->DrawClone("colz");
    c0->cd(3);
    hpnwtrans->DrawClone("colz");
    c0->cd(4);
    hExBefore->SetLineColor(kRed);
    hExBefore->DrawNormalized("histe");
    hExAfter->SetLineColor(kGreen);
    hExAfter->DrawNormalized("histe same");
    gPad->BuildLegend();
    c0->cd(5);
    hkin->DrawClone("colz");
    c0->cd(6);
    // hthetas->DrawClone("colz");
    htwoes->DrawClone("colz");


    // CM canvas
    auto* c1 {new TCanvas {"c1", "CM Canvas"}};
    c1->DivideSquare(4);
    c1->cd(1);
    hpncm->DrawClone("colz");
    c1->cd(2);
    hpnwcm->DrawClone("colz");
    c1->cd(3);
    hpnwtranscm->DrawClone("colz");
    c1->cd(4);
    hkincm->DrawClone("colz");
}
