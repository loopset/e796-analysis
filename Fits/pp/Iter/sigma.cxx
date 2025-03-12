#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "THStack.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "FitInterface.h"
#include "FitPlotter.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <iostream>
#include <string>
#include <vector>

#include "../../../Selector/Selector.h"
#include "../../FitHist.h"

struct Return
{
    TH1D* fHist {};
    double fEx {};
    double fSigma {};
};

Return Run(ROOT::RDF::RNode ana, ROOT::RDF::RNode simu, double emin, double emax)
{
    // Experimental data
    auto df {ana.Filter([=](double elab) { return emin <= elab && elab <= emax; }, {"EVertex"})};
    auto hEx {df.Histo1D(E796Fit::Expp, "Ex")};
    // Phase space deuton breakup
    auto phase {simu.Filter([=](double elab) { return emin <= elab && elab <= emax; }, {"EVertex"})};
    auto hPS {phase.Histo1D(E796Fit::Expp, "Eex", "weight")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr(), 0);

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
    // Eval sigma from interpolator
    inter.EvalSigma(sigmas.GetGraph());
    // And fix it!
    inter.SetFix("g1", 2, true);
    inter.SetFix("g2", 2, true);
    inter.SetFix("g3", 2, true);
    inter.SetFix("g4", 2, true);
    // inter.SetFix("ps0", 0, true);
    // inter.Print();

    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS}, inter.GetCte()};

    // Fitting range
    double exmin {-10};
    double exmax {12};

    // Run!
    auto h {hEx.GetPtr()};
    Fitters::Data data {*h, exmin, exmax};

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

    // Draw
    gPad->cd();
    // Configure global Ex histogram
    h->SetTitle(TString::Format("E_{lab} #in [%.2f, %.2f] MeV", emin, emax));
    h->SetStats(false);
    h->GetXaxis()->SetRangeUser(exmin, exmax);
    h->SetLineWidth(2);
    auto* clone = h->DrawClone("e");
    gfit->Draw("same");
    auto* stack {new THStack};
    // Set fill styles
    std::vector<int> fills {3345, 3354, 3344};
    int idx {};
    for(auto& [key, h] : hfits)
    {
        TString rstr {key};
        bool isPS {rstr.Contains("ps")};
        h->SetLineWidth(2);
        h->SetFillStyle(0);
        if(idx < fills.size() && isPS)
        {
            h->SetFillStyle(fills[idx]);
            idx++;
        }
        stack->Add(h);
    }
    stack->Draw("nostack plc pfc same");

    // Return values
    return {.fHist = (TH1D*)h->Clone(), .fEx = res.Parameter(1), .fSigma = res.Parameter(2)};
}


void sigma()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame ana {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "1H")};
    ROOT::RDataFrame simu {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -1, 0)};

    // Define intervals to fit
    std::vector<std::pair<double, double>> ivs {};
    double emin {2.8};
    double emax {6.3};
    int n {4};
    double step {(emax - emin) / n};
    for(int i = 0; i < n; i++)
    {
        auto low {emin + i * step};
        auto up {emin + (i + 1) * step};
        ivs.push_back({low, up});
        // std::cout << "i : " << i << " min : " << low << " max : " << up << '\n';
    }

    // TGraphErrors to save
    auto* gex {new TGraphErrors};
    gex->SetTitle("gs Ex variation;E_{lab} [MeV];E_{x} gs [MeV]");
    auto* gsigma {new TGraphErrors};
    gsigma->SetTitle("Sigma variation;E_{lab} [MeV];#sigma_{g.s} [MeV]");
    for(auto* g : {gex, gsigma})
    {
        g->SetLineWidth(2);
        g->SetLineColor(kMagenta);
        g->SetMarkerColor(kMagenta);
        g->SetMarkerStyle(25);
    }
    // Stack
    auto* stack {new THStack};
    stack->SetTitle("Stack of spectra;E_{x} [MeV];Normalized counts");

    // Create canvas and do!
    auto* c0 {new TCanvas {"c0", "Sigma investigation"}};
    c0->DivideSquare(ivs.size());

    for(int i = 0; i < ivs.size(); i++)
    {
        auto& pair {ivs[i]};
        auto center {(pair.second + pair.first) / 2};
        c0->cd(i + 1);
        gPad->SetLogy();
        // DO
        std::cout << "For E in [" << pair.first << ", " << pair.second << "]" << '\n';
        auto ret = Run(ana, simu, pair.first, pair.second);
        gsigma->AddPoint(center, ret.fSigma);
        gex->AddPoint(center, ret.fEx);
        // Push to stack
        ret.fHist->Scale(1. / ret.fHist->Integral());
        ret.fHist->SetLineWidth(2);
        stack->Add(ret.fHist, "histe");
    }

    // Results canvas
    auto* c1 {new TCanvas {"c1", "Results canvas"}};
    c1->DivideSquare(4);
    c1->cd(1);
    gsigma->Draw("apl");
    c1->cd(2);
    gex->Draw("apl");
    c1->cd(3);
    stack->Draw("nostack plc pmc");
    gPad->BuildLegend();
}
