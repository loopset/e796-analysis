#include "ActKinematics.h"

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
#include "uncertainties.hpp"
#include "ureal.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../../Selector/Selector.h"
#include "../../FitHist.h"

struct Return
{
    TH1D* fHist {};
    unc::udouble fEx {};
    unc::udouble fSigma {};
    unc::udouble fGamma {};
};

void FillGraph(TGraphErrors* g, double x, const unc::udouble& res)
{
    g->AddPoint(x, res.n());
    g->SetPointError(g->GetN() - 1, 0, res.s());
}

Return Run(ROOT::RDF::RNode ana, double emin, double emax)
{
    // Experimental data
    auto df {ana.Filter([=](double elab) { return emin <= elab && elab <= emax; }, {"EVertex"})};
    auto hEx {df.Histo1D(E796Fit::Exdd, "Ex")};

    // Sigmas
    Interpolators::Sigmas sigmas;
    sigmas.Read(gSelector->GetSigmasFile("2H", "2H").Data());

    // Init interface
    Fitters::Interface inter;
    double sigma {0.3}; // common init sigma for all
    // Key for gs
    std::string gs {"g0"};
    inter.AddState(gs, {400, 0, sigma}, "0+");
    inter.AddState("g1", {100, 1.6, sigma}, "2+0");
    inter.AddState("g2", {50, 4, sigma}, "2+0");
    inter.AddState("g3", {50, 5.5, sigma}, "3-");
    // inter.AddState("g4", {50, 6.5, sigma}, "2+");
    // inter.AddState("g5", {50, 7.6, sigma}, "3- and 4+");
    // inter.AddState("g6", {50, 8.6, sigma}, "4+0");
    // inter.AddState("g7", {50, 9.6, sigma}, "0+2");
    inter.EndAddingStates();
    // Sigma from interpolator
    inter.EvalSigma(sigmas.GetGraph());
    inter.SetFixAll(2, true);
    // inter.SetBoundsAll(3, {0.01, 0.5});
    inter.SetFix(gs, 2, false); // release for gs
    // inter.SetFixAll(3, true);

    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {}, inter.GetCte()};

    // Fitting range
    double exmin {-5};
    double exmax {6.5};

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
    // Locate parameters
    std::vector<int> idxs;
    for(auto str : {"_Mean", "_Sigma", "_Lg"})
    {
        auto key {gs + str};
        int i {};
        for(int p = 0; p < res.NPar(); p++)
        {
            auto name {res.ParName(p)};
            if(name == key)
            {
                idxs.push_back(p);
                break;
            }
        }
    }
    auto gamma {unc::udouble {}};
    if(idxs.size() != 3)
        gamma = {-1, 0};
    else
        gamma = {res.Parameter(idxs[2]), res.Error(idxs[2])};

    // Return values
    return {.fHist = (TH1D*)h->Clone(),
            .fEx = {res.Parameter(idxs[0]), res.Error(idxs[0])},
            .fSigma = {res.Parameter(idxs[1]), res.Error(idxs[1])},
            .fGamma = gamma};
}


void sigma()
{
    std::string target {"2H"};
    std::string light {"2H"};

    ROOT::EnableImplicitMT();
    ROOT::RDataFrame ana {"Sel_Tree", gSelector->GetAnaFile(3, "20O", target, light)};

    // Define intervals to fit
    std::vector<std::pair<double, double>> ivs {};
    double emin {3.5};
    double emax {9.2};
    int n {6};
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
    auto* ggamma {new TGraphErrors};
    ggamma->SetTitle("#Gamma variation;E_{lab} [MeV];#Gamma_{g.s} [MeV]");
    for(auto* g : {gex, gsigma, ggamma})
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
        auto ret = Run(ana, pair.first, pair.second);
        // Ex
        FillGraph(gex, center, ret.fEx);
        // Sigma
        FillGraph(gsigma, center, ret.fSigma);
        // Gamma
        FillGraph(ggamma, center, ret.fGamma);
        // Push to stack
        ret.fHist->Scale(1. / ret.fHist->Integral());
        ret.fHist->SetLineWidth(2);
        if(i == 0)
        {
            ret.fHist->Fit("gaus", "0Q+", "", -0.9, 0.9);
            ret.fHist->GetFunction("gaus")->SetBit(TF1::kNotDraw);
        }
        stack->Add(ret.fHist, "e");
    }

    // Results canvas
    auto* c1 {new TCanvas {"c1", "Results canvas"}};
    c1->DivideSquare(4);
    c1->cd(1);
    gsigma->Draw("apl");
    c1->cd(2);
    gex->Draw("apl");
    c1->cd(3);
    ggamma->Draw("apl");
    c1->cd(4);
    stack->Draw("nostack plc pmc");
    for(auto* h : *stack->GetHists())
        for(auto* o : *((TH1D*)h)->GetListOfFunctions())
            if(o)
                o->Draw("same");
    gPad->BuildLegend();
}
