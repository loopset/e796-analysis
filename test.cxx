#include "ActKinematics.h"

#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TMath.h"

#include <cmath>
void test()
{
    ActPhysics::Kinematics k {"20O(p,d)@700"};
    auto* gtheo {k.GetThetaLabvsThetaCMLine()};
    // Limit angles!
    auto itmax {TMath::LocMax(gtheo->GetN(), gtheo->GetY())};
    auto labmax {gtheo->GetPointY(itmax) - 1};
    auto cmmax {gtheo->GetPointX(itmax) - 1};

    // CM to Lab. we need y axis = cm and x axis = lab
    auto* g {new TGraphErrors};
    for(int p = 0; p < gtheo->GetN(); p++)
    {
        auto lab {gtheo->GetPointY(p)};
        auto cm {gtheo->GetPointX(p)};
        if(cm <= cmmax)
        {
            // Apply cos formula
            lab = TMath::Cos(lab * TMath::DegToRad());
            cm = TMath::Cos(cm * TMath::DegToRad());
            // Fill
            g->AddPoint(lab, cm);
        }
    }
    g->SetTitle("Cos plot;cos lab;cos cm");
    // Get derivative
    auto* func {new TF1 {"func", [=](double* x, double* p) { return g->Eval(x[0], nullptr, "S"); }, -1, 1, 0}};
    auto* gder {new TGraphErrors};
    for(int p = 0; p < g->GetN(); p++)
    {
        auto x {g->GetPointX(p)};
        auto derivate {func->Derivative(x)};
        if(std::isfinite(derivate))
            gder->AddPoint(x, derivate);
    }
    gder->SetTitle("derivative;cos lab;derivative");

    // Read lab and cm cross sections
    auto* gcm {new TGraphErrors {"./Fits/pd/Inputs/g0_2FNR/21.g0", "%lg %lg"}};
    gcm->SetLineColor(8);
    auto* glab {new TGraphErrors {"./Fits/pd/Inputs/g0_2FNR/24.g0", "%lg %lg"}};
    // Try to reconstruct it!
    auto* gtest {new TGraphErrors};
    gtest->SetLineColor(kRed);
    for(double lab = 0; lab < labmax; lab += 0.1)
    {
        // Eval derivative
        auto cos {TMath::Cos(lab * TMath::DegToRad())};
        auto der {gder->Eval(TMath::Cos(lab * TMath::DegToRad()))};
        // Transform to cm
        auto cm {TMath::ACos(g->Eval(cos)) * TMath::RadToDeg()};
        auto xs {gcm->Eval(cm)};
        auto trans {xs * der};
        gtest->AddPoint(lab, trans);
    }

    for(auto& g : {gcm, glab, gtest})
    {
        g->SetLineWidth(2);
    }

    // Test kinematics works
    auto* gkin {k.TransfromCMCrossSectionToLab(gcm)};
    gkin->SetLineColor(kMagenta);

    auto* c0 {new TCanvas {"c0", "Kinematic tests"}};
    c0->DivideSquare(4);
    c0->cd(1);
    gtheo->Draw("apl");
    c0->cd(2);
    g->Draw("apl");
    c0->cd(3);
    gder->Draw("apl");
    c0->cd(4);
    gcm->Draw("al");
    glab->Draw("l");
    // gtest->Draw("l");
    gkin->Draw("l");
}
