#include "ActKinematics.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TROOT.h"
#include "TStyle.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <memory>
#include <vector>

#include "../../../PostAnalysis/HistConfig.h"
#include "../../../Selector/Selector.h"

void pd_cont()
{
    gStyle->SetOptFit();
    // Transform kinematics
    double Ex {16.20}; // in (d,t) channel
    ActPhysics::Kinematics dt {"20O(d,t)@700|16.2"};
    // (p,d) gs
    ActPhysics::Kinematics pd {"20O(p,d)@700"};

    // Transform
    auto* gtrans {new TGraphErrors};
    gtrans->SetTitle("CM trans;#theta_{CM} (d,t);#theta_{CM} (p,d)");
    gtrans->SetMarkerStyle(24);
    gtrans->SetLineWidth(2);
    double min {1}; // Lab
    double max {30};
    double step {1};
    auto* gdt {dt.GetThetaLabvsThetaCMLine()};
    auto* ginvdt {new TGraph};
    ginvdt->SetTitle("(d,t) @ 16.2 MeV;Lab;CM");
    auto* gpd {pd.GetThetaLabvsThetaCMLine()};
    auto* ginvpd {new TGraph};
    ginvpd->SetTitle("(p,d) gs;Lab;CM");
    // Invert TGraph so we can get LAB angle in X axis instead of Y
    for(int p = 0; p < gdt->GetN(); p++)
    {
        auto cm {gdt->GetPointX(p)};
        auto lab {gdt->GetPointY(p)};
        if(cm < 40)
            ginvdt->AddPoint(lab, cm);
    }
    for(int p = 0; p < gpd->GetN(); p++)
    {
        auto cm {gpd->GetPointX(p)};
        auto lab {gpd->GetPointY(p)};
        if(cm < 40)
            ginvpd->AddPoint(lab, cm);
    }
    for(double t = min; t <= max; t += step)
    {
        auto cmdt {ginvdt->Eval(t)};
        auto cmpd {ginvpd->Eval(t)};
        gtrans->AddPoint(cmdt, cmpd);
    }
    gtrans->Fit("pol1", "0QM");
    gtrans->GetFunction("pol1")->ResetBit(TF1::kNotDraw);

    // Reconstruct xs from counts
    // Counts
    auto cfile {std::make_unique<TFile>("../Outputs/rebin/counts.root")};
    auto* gcounts {cfile->Get<TGraphErrors>("gv8")};
    // Transform gcounts thetaCM
    auto* gclone {(TGraphErrors*)gcounts->Clone()};
    for(int p = 0; p < gcounts->GetN(); p++)
    {
        gcounts->SetPointX(p, gtrans->Eval(gcounts->GetPointX(p)));
    }
    // ThetaCM intervals following our transformation
    auto* fOmega {new TF1 {"fOmega", "TMath::TwoPi() * TMath::Sin(x)", 0, TMath::Pi()}};
    std::vector<std::pair<double, double>> ranges;
    std::vector<double> omegas {};
    auto thetaMin {gcounts->GetPointX(0)};
    auto thetaStep {gcounts->GetPointX(1) - thetaMin};
    for(int p = 0; p < gcounts->GetN(); p++)
    {
        auto x {gcounts->GetPointX(p)};
        auto low {x - thetaStep / 2};
        auto up {x + thetaStep / 2};
        ranges.push_back({low, up});
        auto integral {fOmega->Integral(low * TMath::DegToRad(), up * TMath::DegToRad())};
        omegas.push_back(integral);
    }
    thetaMin -= thetaStep / 2;
    Angular::Intervals ivs {thetaMin, thetaMin + thetaStep * (gcounts->GetN() - 1), HistConfig::Ex, thetaStep};
    // But overwrite with manual ranges
    ivs.SetRanges(ranges);
    ivs.SetOmegas(omegas);
    PhysUtils::Experiment exp {"../../norms/d_target.dat"};
    // But EFFICIENCY from (p,d) gs!!
    Interpolators::Efficiency eff;
    eff.Add("v8", gSelector->GetApproxSimuFile("20O", "1H", "2H", 0));
    // XS
    Angular::DifferentialXS xs {&ivs, &eff, &exp};
    xs.DoFor(gcounts, "v8");
    auto* gxs {xs.Get("v8")};

    // Comparator
    Angular::Comparator comp {"v8 as (p,d) gs", gxs};
    comp.Add("l = 0", "../../pd/Inputs/g0_FRESCO/fort.202");
    comp.Add("l = 1", "../../pd/Inputs/g0_FRESCO/fort.203");
    comp.Add("l = 2", "../../pd/Inputs/g0_FRESCO/fort.204");
    comp.Fit();
    comp.Draw();

    auto* c0 {new TCanvas {"c0", "Contamination canvas"}};
    c0->DivideSquare(6);
    c0->cd(1);
    ginvdt->Draw("apl");
    c0->cd(2);
    ginvpd->Draw("apl");
    c0->cd(3);
    gtrans->Draw("apl");
    c0->cd(4);
    gcounts->Draw("apl");
    c0->cd(5);
    gxs->Draw("apl");
    c0->cd(6);
    fOmega->Draw();
    // gclone->Draw("pl");
    // gxs->Draw("ap");
    // gtransxs->Draw("p");
}
