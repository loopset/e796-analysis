#include "ActKinematics.h"
#include "ActSRIM.h"

#include "TGraphErrors.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TVirtualPad.h"

#include <iostream>

void doKin(double thetaCM, double fEcm, double fBeta, double fm3, double fm4, double fEx = 0)
{
    // Set angles
    double fThetaCM {thetaCM};
    double fPhiCM {0};
    // Compute kinematics in CM for 3rd particle
    double E3CM {0.5 * (fEcm * fEcm + fm3 * fm3 - (fm4 + fEx) * (fm4 + fEx)) / fEcm};
    double p3CM {TMath::Sqrt(E3CM * E3CM - fm3 * fm3)};
    TLorentzVector P3CM = {p3CM * TMath::Cos(fThetaCM), p3CM * TMath::Sin(fThetaCM) * TMath::Sin(fPhiCM),
                           p3CM * TMath::Sin(fThetaCM) * TMath::Cos(fPhiCM), E3CM};
    std::cout << "thetaCM : " << fThetaCM << '\n';
    std::cout << "E3CM : " << E3CM << '\n';
    std::cout << "p3CM : " << p3CM << '\n';
    P3CM.Print();
}

void srimbea()
{
    ActPhysics::Kinematics k {"238U(24Mg,20Ne)@1371|0"};
    doKin(180 * TMath::DegToRad(), k.GetECM(), k.GetBeta(), k.GetMass(3), k.GetMass(4));
    // ActPhysics::SRIM srim;
    // srim.ReadTable("target", "./Calibrations/SRIMData/raw/24Mg_Mg.txt");
    // srim.ReadTable("al", "./Calibrations/SRIMData/raw/24Mg_Al.txt");
    // srim.ReadTable("si", "./Calibrations/SRIMData/raw/24Mg_Si.txt");
    // srim.ReadTable("au", "./Calibrations/SRIMData/raw/24Mg_Au.txt");
    //
    // auto* mg {new TMultiGraph};
    //
    // // Thicknesses (mm)
    // double target {2.88e-3 / 2};
    // double al {0.1e-3};
    // double si {70e-3};
    // double au {0.3e-3};
    //
    // double ini {50};
    // double end {300};
    // double step {1};
    // for(double t = 0; t <= 50; t += 10)
    // {
    //     auto theta {t * TMath::DegToRad()};
    //
    //     auto* g {new TGraphErrors};
    //     g->SetTitle(TString::Format("%.2f#circ;E_{res} [MeV];#Delta E [MeV]", t));
    //     for(double it = ini; it <= end; it += step)
    //     {
    //         auto e {it};
    //         // Target
    //         e = srim.Slow("target", e, target, theta);
    //         // Al
    //         e = srim.Slow("al", e, al, theta);
    //         // Si0
    //         double EIn0 {e};
    //         e = srim.Slow("si", e, si, theta);
    //         auto deltae {EIn0 - e};
    //         // Au
    //         e = srim.Slow("au", e, au, theta);
    //         // Al again
    //         e = srim.Slow("al", e, al, theta);
    //         // Si1
    //         auto EIn1 {e};
    //         e = srim.Slow("si", e, si, theta);
    //         auto eres {EIn1 - e};
    //         g->AddPoint(eres, deltae);
    //     }
    //     mg->Add(g);
    // }
    //
    // mg->Draw("apl plc pmc");
    // gPad->BuildLegend();
}
