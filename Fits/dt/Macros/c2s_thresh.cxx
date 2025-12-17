#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TMath.h"
#include "TRandom.h"
#include "TString.h"

#include "AngComparator.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <iostream>
#include <vector>
void c2s_thresh()
{
    // Read fit
    auto* fit {new TFile("../Outputs/fit_juan_RPx.root")};
    auto* list {fit->Get<TList>("HistoPeaks")};
    auto* hps {(TH1D*)list->FindObject("hps0")};
    hps->SetDirectory(nullptr);

    auto* gaus {new TF1 {"funcgaus", "TMath::Gaus(x, 15, 0.35)", 0, 30}};

    std::vector<TH1D*> hs;
    double minc {1};
    double maxc {50};
    double step {3};
    for(double c = minc; c <= maxc; c += step)
    {
        auto* hit {(TH1D*)hps->Clone("hit")};
        hit->SetDirectory(nullptr);
        hit->Reset();
        hit->SetTitle(TString::Format("C = %d", (int)c));
        hit->FillRandom("funcgaus");
        hit->Scale(c / hit->Integral());
        hit->Add(hps);
        hs.push_back(hit);
    }

    auto* hchi {new TH1D {"hchi", "Chi histo", 200, 0, 1}};
    int idx {};
    for(const auto& h : hs)
    {
        auto* fitfunc {new TF1 {TString::Format("fitfunc%d", idx),
                                [&](double* x, double* p)
                                {
                                    auto gaus {p[0] * TMath::Gaus(x[0], p[1], p[2])};
                                    auto ps {hps->Interpolate(x[0])};
                                    return gaus + ps;
                                },
                                0, 30, 3}};
        fitfunc->SetParameters(0, 15, 0);
        fitfunc->FixParameter(2, 0.35);
        auto lims {5}; // MeV
        auto mean {15};
        fitfunc->SetParLimits(1, mean - lims, mean + lims);
        fitfunc->SetParLimits(0, 0, 1e5);
        h->Fit(fitfunc, "0Q+");
        auto chi {fitfunc->GetChisquare()};
        hchi->Fill(chi);

        idx++;
    }

    // Read theoretical xs
    auto* gexp {new TGraphErrors {"../Outputs/xs/v7_xs.dat", "%lg %lg %lg"}};
    Angular::Comparator comp {"15 MeV", gexp};
    comp.Add("l = 1", "../Inputs/Daeh/ex_15/fort.203");
    comp.Fit();
    auto theo {comp.IntegralModel("l = 1")};

    // Efficiency
    Interpolators::Efficiency eff;
    eff.Add("15 MeV", "../../../Simulation/Outputs/juan_RPx/tree_20O_2H_3H_15.00_nPS_0_pPS_0.root");
    auto xsrange {comp.GetFitRange()};
    auto meaneff {eff.GetMeanEff("15 MeV", xsrange.first, xsrange.second)};

    // Experiment
    PhysUtils::Experiment exp {"../../norms/d_target.dat"};
    exp.Print();
    auto ntheo {theo * 1e-27 * exp.GetNb() * exp.GetNt() * meaneff};
    std::cout << "Theo xs : " << theo << '\n';
    std::cout << "Mean eff: " << meaneff << '\n';
    std::cout << "Theo N  : " << ntheo << '\n';
    std::cout << "Min C2S : " << 22. / ntheo << '\n';
    // where 22 is the min number of counts for the fit to identify the gaussian peak,
    // determined from observation of c1 canvas

    // Draw
    auto* c0 {new TCanvas {"c0", "c2s thresh"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hps->Draw();
    c0->cd(2);
    hchi->Draw();

    auto* c1 {new TCanvas {"c1", "Added canvas"}};
    c1->DivideSquare(hs.size());
    for(int i = 0; i < hs.size(); i++)
    {
        c1->cd(i + 1);
        hs[i]->Draw("hist");
        for(auto* o : *(hs[i]->GetListOfFunctions()))
            if(o)
                o->DrawClone("same");
    }
}
