#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TString.h"
#include "TStyle.h"
#include "TVirtualPad.h"

#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <utility>
#include <vector>

std::pair<double, double> Integral(TH1D* h, double exmin, double exmax)
{
    auto bmin {h->FindBin(exmin)};
    auto bmax {h->FindBin(exmax)};
    auto i {h->Integral(bmin, bmax)};
    return {i, TMath::Sqrt(i)};
}

void Ang_ps()
{
    Angular::Fitter res {};
    res.Read("../Outputs/fitter.root");
    res.Draw();
    // Extract objects
    auto ivs {res.GetIvs()};
    auto all {res.GetResHistos()};

    // Get PS histograms
    std::vector<TH1D*> ps {ivs->GetHistos()};
    // for(int i = 0; i < all.size(); i++)
    // {
    //     for(const auto& h : all[i])
    //     {
    //         if(TString(h->GetName()).Contains("ps0"))
    //         {
    //             ps.push_back((TH1D*)h->Clone());
    //             auto title {TString::Format("d-break %s", ivs->GetHistos()[i]->GetTitle())};
    //             ps.back()->SetTitle(title);
    //         }
    //     }
    // }

    // Compute cross-section per cut in Ex
    PhysUtils::Experiment exp {"../../norms/d_target.dat"};
    Interpolators::Efficiency eff {"ps0", "../../../Simulation/Outputs/juan_RPx/tree_20O_2H_2H_0.00_nPS_-1_pPS_0.root"};
    eff.Draw();
    // Get counts for intervals in Ex
    std::vector<std::pair<double, double>> exs {{-10, -5}, {-5, -3}, {-3, -1.}, {8.5, 10.5}};
    std::vector<TGraphErrors*> counts {};
    std::vector<TGraphErrors*> xss {};
    for(const auto& pair : exs)
    {
        counts.push_back(new TGraphErrors);
        auto& g {counts.back()};
        for(int i = 0; i < ps.size(); i++)
        {
            auto [v, u] {Integral(ps[i], pair.first, pair.second)};
            g->AddPoint(ivs->GetCenter(i), v);
            g->SetPointError(g->GetN() - 1, 0, u);
        }
        // Do computation
        Angular::DifferentialXS xs {ivs, &eff, &exp};
        xs.DoFor(g, "ps0");
        auto* out {xs.Get("ps0")};
        // Redefine x axis
        out->GetXaxis()->SetTitle("cos(#theta_{CM})");
        for(int p = 0; p < out->GetN(); p++)
            out->SetPointX(p, TMath::Cos(out->GetPointX(p) * TMath::DegToRad()));
        // And also Y axis
        out->GetYaxis()->SetTitle("d#sigma/d#Omega #times #Delta#Omega [mb]");
        for(int p = 0; p < out->GetN(); p++)
        {
            auto cs {out->GetPointY(p)};
            auto omega {ivs->GetOmega(p)};
            out->SetPointY(p, cs * omega);
        }
        out->SetTitle(TString::Format("E_{x} #in [%.1f, %.1f] MeV", pair.first, pair.second));
        xss.push_back(out);
    }


    // Fit to cos!
    std::vector<TF1*> fits;
    for(auto* g : xss)
    {
        // auto* fit {new TF1 {"fit", "[0] + [1] * TMath::Cos(x * TMath::DegToRad())", 10, 40}};
        auto* fit {new TF1 {"fit", "[0] + [1] * x", 0, 1}};
        fit->SetParameters(1, 1);
        fit->SetParNames("Offset", "Slope");
        g->Fit(fit, "0QM+");
        fits.push_back(fit);
    }

    // Set styles
    for(auto* g : counts)
    {
        g->SetLineColor(kMagenta);
        g->SetMarkerColor(kMagenta);
        g->SetLineWidth(2);
        g->SetMarkerStyle(24);
    }
    for(auto* g : xss)
    {
        g->SetLineColor(9);
        g->SetMarkerColor(9);
        g->SetLineWidth(2);
        g->SetMarkerStyle(25);
    }

    // Draw
    gStyle->SetOptFit(true);
    auto* c0 {new TCanvas {"c0", "d-breakup canvas"}};
    c0->DivideSquare(ps.size());
    for(int i = 0; i < ps.size(); i++)
    {
        c0->cd(i + 1);
        gPad->SetLogy();
        ps[i]->SetLineWidth(1);
        ps[i]->Draw();
    }
    auto* c1 {new TCanvas {"c1", "ps xs"}};
    c1->DivideSquare(4);
    for(int i = 0; i < xss.size(); i++)
    {
        c1->cd(i + 1);
        xss[i]->Draw("apl");
        fits[i]->Draw("same");
    }
}
