#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "THStack.h"
#include "TPaveText.h"
#include "TROOT.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "FitInterface.h"
#include "FitModel.h"
#include "FitPlotter.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <iostream>
#include <string>
#include <vector>

#include "../../../Selector/Selector.h"
#include "/media/Data/E796v2/Fits/FitHist.h"

struct GlobalRes
{
    double fChi2red {};
    TGraph* fGlobal {};
    TH1D* hPS {};
};

GlobalRes DoGlobalFit(const Fitters::Interface& inter, TH1D* hEx, TH1D* hPS, double exmin, double exmax)
{
    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS}, inter.GetCte()};
    // Init data
    Fitters::Data data {*hEx, exmin, exmax};
    // Init runner
    Fitters::Runner runner {data, model};
    runner.GetObjective().SetUseDivisions(true);
    // And initial parameters
    runner.SetInitial(inter.GetInitial());
    runner.SetBounds(inter.GetBounds());
    runner.SetFixed(inter.GetFixed());
    // Run
    runner.Fit();

    // Get fit result
    auto res {runner.GetFitResult()};
    // Plotter
    Fitters::Plotter plot {&data, &model, &res};
    auto* gfit {plot.GetGlobalFit()};
    auto hfits {plot.GetIndividualHists()};
    auto fit {hfits["ps0"]};
    fit->SetLineWidth(2);
    fit->SetLineColor(8);
    return {.fChi2red = res.Chi2() / res.Ndf(), .fGlobal = gfit, .hPS = fit};
}

void pscorr()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "1H")};
    // Ex
    auto hEx {df.Histo1D(E796Fit::Expp, "Ex")};
    // Phase space
    ROOT::RDataFrame phase {"SimulationTTree", "../../../Simulation/Macros/Breakup/Outputs/d_breakup_trans.root"};

    // Sigmas
    Interpolators::Sigmas sigmas;
    sigmas.Read(gSelector->GetSigmasFile("1H", "1H").Data());

    // Init interface
    Fitters::Interface inter;
    inter.AddState("g0", {400, 0, 0.3}, "0+");
    inter.AddState("g1", {100, 1.67, 0.3}, "2+");
    inter.AddState("g2", {50, 4.1, 0.3}, "2+");
    inter.AddState("g3", {50, 5.6, 0.3}, "3-");
    inter.AddState("g4", {50, 7.8, 0.3}, "3- and 4+");
    inter.AddState("ps0", {0.455});
    inter.EndAddingStates();
    // Read previous fit
    inter.ReadPreviousFit("../Outputs/fit_juan_RPx.root");
    // Eval sigma from interpolator
    inter.EvalSigma(sigmas.GetGraph());
    // And fix it!
    inter.SetFix("g1", 2, true);
    inter.SetFix("g2", 2, true);
    inter.SetFix("g3", 2, true);
    inter.SetFix("g4", 2, true);
    // Fitting range
    double exmin {-15};
    double exmax {15};

    std::vector<double> cutoffs {};
    for(double c = 100; c <= 250; c += 10)
        cutoffs.push_back(c);

    std::vector<TH1D*> hpss {};
    auto* stack {new THStack};
    stack->SetTitle("PS with cutoff;Ex;Counts");
    // Chi2
    auto* gchi {new TGraphErrors};
    gchi->SetTitle("Chi2 vs cutoff;Cutoff;Chi2red");
    // Fitted PS
    std::vector<TH1D*> hfitpss, hfitex;
    std::vector<TGraph*> gglobals;
    for(const auto& cutoff : cutoffs)
    {
        // Redefine weight
        auto node {phase.Redefine("weight_trans",
                                  [&](double w, double deltap)
                                  {
                                      if(deltap > cutoff)
                                          return 0.;
                                      else
                                          return w * TMath::Power(1 - TMath::Power(deltap / cutoff, 1), 1);
                                  },
                                  {"weight", "DeltaPCM"})};
        auto hPS {node.Histo1D(E796Fit::Expp, "Eex", "weight_trans")};
        // Treat
        Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr(), 2, 0.2);

        hpss.push_back((TH1D*)hPS->Clone());
        hpss.back()->SetTitle(TString::Format("Cutoff = %.2f", cutoff));
        stack->Add(hpss.back());

        auto [chi, global, fitps] {DoGlobalFit(inter, hEx.GetPtr(), hPS.GetPtr(), exmin, exmax)};
        gchi->AddPoint(cutoff, chi);
        gglobals.push_back(global);
        hfitpss.push_back(fitps);
        auto* clone {(TH1D*)hEx->Clone()};
        clone->SetTitle(TString::Format("Cutoff = %.2f", cutoff));
        hfitex.push_back(clone);
    }

    // Find minimum
    TF1 fchi {"fchi", [&](double* x, double* p) { return gchi->Eval(x[0], nullptr, "S"); }, cutoffs.front(),
              cutoffs.back(), 0};
    auto min {fchi.GetMinimumX()};
    std::cout << "Cutoff : " << min << " chi2red : " << fchi.Eval(min) << '\n';
    auto* text {new TPaveText {0.6, 0.6, 0.9, 0.9, "ndc"}};
    text->AddText(TString::Format("Min = %.2f", min));
    gchi->GetListOfFunctions()->Add(text);

    // Draw
    auto* c0 {new TCanvas {"c0", "PS canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    stack->Draw("nostack plc pmc");
    gPad->BuildLegend();
    c0->cd(2);
    gchi->SetLineWidth(2);
    gchi->SetMarkerStyle(24);
    gchi->Draw("apl");

    auto* c1 {new TCanvas {"c1", "Fitted ps"}};
    c1->DivideSquare(hfitpss.size());
    for(int i = 0; i < hfitpss.size(); i++)
    {
        c1->cd(i + 1);
        // gPad->SetLogy();
        auto& clone {hfitex[i]};
        clone->Draw();
        clone->GetYaxis()->SetRangeUser(1, 500);
        gPad->SetLogy();
        gglobals[i]->Draw("l");
        auto& fit {hfitpss[i]};
        fit->SetLineColor(9);
        fit->SetLineWidth(2);
        fit->Draw("same");
    }
}
