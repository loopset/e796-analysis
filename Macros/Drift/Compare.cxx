#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TH2.h"
#include "TMarker.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TObject.h"
#include "TPaveText.h"
#include "TProfile.h"
#include "TString.h"
#include "TTree.h"
#include "TVirtualPad.h"

#include "uncertainties.hpp"
#include "ureal.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../../Selector/Selector.h"

double func_offset_constrained(double* x, double* p)
{
    auto z {x[0]};
    auto a {p[0]};
    auto b {p[1]};
    auto c {p[2]};
    double zb {169.25}; // mm, determined from AntiVetoSM->GetCenter({3,4}) + beamOffset calculated for simu
    return a + b * (z - zb) + c * TMath::Power(z - zb, 2);
}

std::pair<double, double> FindExtreme(TF1* func)
{
    auto p2 {func->GetParameter(2)};
    // Maxima or minima
    double extreme {};
    double pos {};
    if(p2 > 0)
    {
        extreme = func->GetMinimum();
        pos = func->GetMinimumX();
    }
    else
    {
        extreme = func->GetMaximum();
        pos = func->GetMaximumX();
    }
    return {pos, extreme};
}

void Compare()
{
    // List values
    const double conv {1. / 4 / 0.08};
    // std::vector<double> drifts {2.144, 2.344, 2.544};
    std::vector<double> drifts;
    for(double d = 2.3; d <= 2.65; d += 0.05)
    {
        drifts.push_back(d);
        std::cout << "vdrift : " << d * conv << '\n';
    }
    drifts.push_back(2.384); // to leave everything where we started
    // Sort values
    std::sort(drifts.begin(), drifts.end());
    std::map<std::string, std::string> names {{"pd", "1H_2H"}, {"dt", "2H_3H"}};

    // Read data
    std::map<std::string, std::vector<TH2D*>> hs2d;
    std::map<std::string, std::vector<TH1D*>> hsex;
    std::map<std::string, std::vector<TProfile*>> hsp;
    std::map<std::string, std::vector<TF1*>> funcs;
    std::map<std::string, TMultiGraph*> mgs;
    for(const auto& [key, name] : names)
    {
        mgs[key] = new TMultiGraph;
        mgs[key]->SetTitle(key.c_str());
        for(const auto& drift : drifts)
        {
            auto fullname {"./Outputs/hs_20O_" + name + "_" + gSelector->GetFlag().c_str() + "_drift_" +
                           TString::Format("%.3f", drift).Data() + ".root"};
            auto file {std::make_unique<TFile>(fullname.c_str())};
            auto* h2d {file->Get<TH2D>("hExZ")};
            auto* hp {file->Get<TProfile>("hProfX")};
            auto* hex {file->Get<TH1D>("hEx")};
            auto* func {hp->GetFunction("pol2")};
            // Refit using new function
            auto* fok {new TF1 {"fok", func_offset_constrained, 0, 350, 3}};
            fok->FixParameter(0, 0);
            hp->Fit(fok, "0Q+");
            auto* fitfunc {hp->GetFunction(fok->GetName())};
            fitfunc->ResetBit(TF1::kNotDraw);
            fitfunc->SetLineStyle(2);
            // But do not use it

            auto* g {new TGraph {func}};
            g->SetLineWidth(2);
            // Null dirs for histograms
            h2d->SetDirectory(nullptr);
            hp->SetDirectory(nullptr);
            // Titles for all
            auto vdrift {drift * conv};
            auto title {TString::Format("%.2f mm/#mus", vdrift)};
            h2d->SetTitle((key + " ").c_str() + title);
            hp->SetTitle((key + " ").c_str() + title);
            g->SetTitle(title);
            func->SetTitle(title);
            if(hex)
            {
                hex->SetDirectory(nullptr);
                hex->SetTitle((key + " ").c_str() + title);
                hsex[key].push_back(hex);
            }
            // Add to structures
            hs2d[key].push_back(h2d);
            hsp[key].push_back(hp);
            mgs[key]->Add(g);
            funcs[key].push_back(func);
        }
    }

    // Find maxima or minima
    std::map<std::string, std::vector<std::pair<double, double>>> extremes;
    for(auto& [key, vec] : funcs)
    {
        int idx {};
        for(auto& func : vec)
        {
            auto [ex, ey] {FindExtreme(func)};
            // Save
            extremes[key].push_back({ex, ey});
            // Add marker to plot
            auto* pl {new TMarker {ex, ey, 25}};
            pl->SetMarkerSize(1.2);
            auto* g {(TGraph*)mgs[key]->GetListOfGraphs()->At(idx)};
            g->GetListOfFunctions()->Add(pl);
            idx++;
        }
    }
    // Correct functions
    // X reference
    int idxref {1};
    std::map<std::string, TMultiGraph*> corrs;
    std::map<std::string, TGraphErrors*> g2s;
    for(auto& [key, vec] : funcs)
    {
        corrs[key] = new TMultiGraph;
        mgs[key]->SetTitle(key.c_str());
        g2s[key] = new TGraphErrors;
        g2s[key]->SetTitle((key + ";v_{d} [mm/#mus];p_{2} [MeV/mm^{2}]").c_str());
        int idx {};
        for(auto& func : vec)
        {
            auto [x, y] {extremes[key][idx]};
            // auto [xref, _] {extremes[key][idxref]};
            double xref {169.25}; // mm
            // 1-> Correct X centering by [3] parameter
            // 2-> Correct Y centering by shifting offset in [4]
            auto diffx {-(x - xref)};
            auto diffy {-(0 - y)}; // reference is g.s
            auto* fcorr {new TF1 {"fcorr", "([0] - [4]) + [1] * (x - [3]) + [2] * TMath::Power(x - [3], 2)",
                                  func->GetXmin(), func->GetXmax()}};
            for(int p = 0; p < func->GetNpar(); p++)
            {
                fcorr->SetParameter(p, func->GetParameter(p));
                fcorr->SetParError(p, func->GetParError(p));
            }
            // Set  X offset!
            fcorr->SetParameter(3, diffx);
            // Set  Y offset!
            fcorr->SetParameter(4, diffy);
            // Create graph
            auto* g {new TGraph(fcorr)};
            g->SetTitle(func->GetTitle());
            g->SetLineWidth(2);
            g->SetLineStyle(2);
            // Add marker to plot new extremes
            auto [ex, ey] {FindExtreme(fcorr)};
            auto* pl {new TMarker {ex, ey, 24}};
            pl->SetMarkerSize(1.2);
            g->GetListOfFunctions()->Add(pl);
            corrs[key]->Add(g);
            // Save p2 parameter in graph
            g2s[key]->AddPoint(drifts[idx] * conv, fcorr->GetParameter(2));
            g2s[key]->SetPointError(g2s[key]->GetN() - 1, 0, fcorr->GetParError(2));
            idx++;
        }
    }

    // Process graphs
    for(auto& [key, g] : g2s)
    {
        g->SetLineWidth(2);
        g->SetMarkerStyle(24);
        g->Fit("pol1", "0QM");
        auto* fit {g->GetFunction("pol1")};
        fit->ResetBit(TF1::kNotDraw);
        // Find root
        unc::udouble a {fit->GetParameter(0), fit->GetParError(0)};
        unc::udouble b {fit->GetParameter(1), fit->GetParError(1)};
        auto root {-a / b};
        auto va {fit->GetParameter(0)};
        auto ua {fit->GetParError(0)};
        auto vb {fit->GetParameter(1)};
        auto ub {fit->GetParError(1)};
        auto unc {TMath::Sqrt(TMath::Power(1. / vb, 2) * ua * ua + TMath::Power(va / vb / vb, 2) * ub * ub)};
        auto* text {new TPaveText {0.55, 0.7, 0.85, 0.85, "NDC"}};
        text->AddText(TString::Format("v_{d} = %.2f", root.n()));
        text->AddText(TString::Format("#pm %.2f mm/#mus", root.s()));
        text->SetBorderSize(0);
        text->SetTextFont(132);
        g->GetListOfFunctions()->Add(text);
        // Print to terminal
        std::cout << "-> Reaction : " << key << '\n';
        std::cout << "   Vdrift   : " << root.format(2) << '\n';
        std::cout << "     Manual unc : " << unc << '\n';
        std::cout << "   DriftF   : " << (root / conv).format(2) << '\n';
    }

    // Save to .root files
    auto fout {std::make_unique<TFile>("../../Publications/analysis/Inputs/drift_corr_funcs.root", "recreate")};
    // (p,d) only
    for(const auto* o : *(corrs["pd"]->GetListOfGraphs()))
        o->Write();
    // p2 graph
    g2s["pd"]->Write("g2");
    fout->Close();


    // Plot
    auto* c0 {new TCanvas {"c0", "Drift correction comparison"}};
    c0->DivideSquare(2);
    c0->cd(1);
    mgs["pd"]->SetTitle("(p,d) drift correction;SP.Z() [mm];E_{gs} [MeV]");
    mgs["pd"]->Draw("al plc");
    gPad->BuildLegend();
    corrs["pd"]->Draw("l plc");
    c0->cd(2);
    mgs["dt"]->SetTitle("(d,t) drift correction;SP.Z() [mm];E_{gs} [MeV]");
    mgs["dt"]->Draw("al plc");
    gPad->BuildLegend();
    corrs["dt"]->Draw("l plc");

    auto* c1 {new TCanvas {"c1", "2D canvas"}};
    int pad {1};
    c1->DivideSquare(hs2d["pd"].size() + hs2d["dt"].size());
    for(const auto& [key, vec] : hs2d)
    {
        for(const auto& h : vec)
        {
            c1->cd(pad);
            h->Draw();
            pad++;
        }
    }
    auto* c2 {new TCanvas {"c2", "Prof canvas"}};
    pad = 1;
    c2->DivideSquare(hsp["pd"].size() + hsp["dt"].size());
    for(const auto& [key, vec] : hsp)
    {
        for(const auto& h : vec)
        {
            c2->cd(pad);
            h->Draw();
            pad++;
        }
    }
    auto* c3 {new TCanvas {"c3", "Ex canvas"}};
    pad = 1;
    c3->DivideSquare(hsex["pd"].size() + hsex["dt"].size());
    for(const auto& [key, vec] : hsex)
    {
        for(const auto& h : vec)
        {
            c3->cd(pad);
            if(h)
                h->Draw();
            pad++;
        }
    }
    auto* c4 {new TCanvas {"c4", "Coefficient canvas"}};
    c4->DivideSquare(2);
    c4->cd(1);
    g2s["pd"]->Draw("apl");
    c4->cd(2);
    g2s["dt"]->Draw("apl");
}
