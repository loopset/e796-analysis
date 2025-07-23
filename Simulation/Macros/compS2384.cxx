#include "ActKinematics.h"
#include "ActSRIM.h"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TVirtualPad.h"

void FillProfile(ActPhysics::SRIM* srim, TGraphErrors* g, double T, double step = 1)
{
    auto range {srim->EvalRange("light", T)};
    double Taux {T};
    for(double d = 0; d <= range; d += step)
    {
        auto Tafter {srim->Slow("light", Taux, step)};
        auto deltaE {Taux - Tafter};
        // To keV
        deltaE *= 1e3;
        Taux = Tafter;
        g->AddPoint(d, deltaE);
    }
}

void compS2384()
{
    // Kinematics
    ActPhysics::Kinematics kin {"11Li(d,t)@700"};
    auto* g3 {kin.GetKinematicLine3()};
    auto t3min {TMath::MinElement(g3->GetN(), g3->GetY())};
    auto t3max {TMath::MaxElement(g3->GetN(), g3->GetY())};

    // SRIM
    auto* srim {new ActPhysics::SRIM};
    srim->SetUseSpline(false);
    srim->ReadTable("light", "../../Calibrations/SRIMData/raw/3H_952mb_mixture.txt");

    auto* mg {new TMultiGraph};
    mg->SetTitle(";d [mm];dE / dx [keV / 1 mm]");

    // For different energies
    t3min = 2;
    t3max = 40;
    auto Tstep {8};
    for(double T = t3min; T <= t3max; T += Tstep)
    {
        std::cout << "T = " << T << '\n';
        auto* g {new TGraphErrors};
        g->SetTitle(TString::Format("T = %.2f", T));
        FillProfile(srim, g, T);
        g->SetLineWidth(2);
        mg->Add(g);
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "S2384 and E796 comp"}};
    c0->DivideSquare(4);
    c0->cd(1);
    mg->SetMinimum(0);
    mg->Draw("al plc pmc");
    gPad->BuildLegend();
}
