#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TH2.h"
#include "THStack.h"
#include "TPaveText.h"
#include "TROOT.h"
#include "TSpline.h"
#include "TString.h"

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "../../../PostAnalysis/HistConfig.h"
#include "../../../Selector/Selector.h"

double Fit(TH1D* h, double ex = 0, double w = 1.5)
{
    h->Fit("gaus", "0QR+", "", ex - w, ex + w);
    auto* fit {h->GetFunction("gaus")};
    if(!fit)
        return 0;
    fit->ResetBit(TF1::kNotDraw);
    return fit->GetParameter(2);
}

double GetOverlap(TH1* ana, TH1* simu)
{
    double chi2 {};
    // Fitting against simulation
    for(int b = 1; b <= simu->GetNbinsX(); b++)
    {
        auto xsimu {simu->GetBinCenter(b)};
        auto ysimu {simu->GetBinContent(b)};
        if(!ysimu)
            continue;
        auto binana {ana->FindBin(xsimu)};
        auto yana {ana->GetBinContent(binana)};
        auto uyana {ana->GetBinError(binana)};
        if(uyana == 0)
            uyana = 1;
        chi2 += std::sqrt(std::pow((yana - ysimu) / uyana, 2));
    }
    return chi2;
}

void plot()
{
    std::string target {"2H"};
    std::string light {"3H"};

    // Read analysis
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame ana {"Sel_Tree", gSelector->GetAnaFile(3, "20O", target, light)};
    auto hEx {ana.Histo1D(HistConfig::Ex, "Ex")};
    auto sgs {Fit(hEx.GetPtr(), 0, 1.5)};
    auto sex {Fit(hEx.GetPtr(), 3.16, 0.75)};
    // Gate on states
    ROOT::RDF::RNode anags {ana.Filter([&](double ex) { return std::abs(ex - 0) <= 3 * sgs; }, {"Ex"})};
    ROOT::RDF::RNode anaex {ana.Filter([&](double ex) { return std::abs(ex - 3.16) <= 3 * sex; }, {"Ex"})};
    std::vector<TH1D*> hsAnaRPx, hsAnaEx;
    for(auto node : {&anags, &anaex})
    {
        auto hRPx {node->Histo1D(HistConfig::RPx, "fRP.fCoordinates.fX")};
        auto hEx {node->Histo1D(HistConfig::Ex, "Ex")};
        hRPx->SetLineColor(8);
        hEx->SetLineColor(8);
        hsAnaRPx.push_back((TH1D*)hRPx->Clone());
        hsAnaEx.push_back((TH1D*)hEx->Clone());
    }

    // Read simulation
    gSelector->SetFlag("iter_front");
    // Set vector of energies
    std::vector<double> Exs {0, 3.24}; // we can compare only gs and excited state at 3 MeV

    // And now distances
    std::vector<double> dists;
    double padSize {2}; // mm
    double pad {256};   // mm
    for(double d = 90; d <= 190; d += 10)
    {
        auto dist {pad + d};
        dists.push_back(dist / padSize);
    }

    // Get histograms
    std::vector<std::vector<TH1D*>> hsRPx;
    std::vector<std::vector<TH2D*>> hsSP;
    for(const auto& dist : dists)
    {
        auto tag {TString::Format("dist_%.2f", dist)};
        gSelector->SetTag(tag.Data());
        hsRPx.push_back({});
        hsSP.push_back({});
        for(const auto& ex : Exs)
        {
            auto file {gSelector->GetApproxSimuFile("20O", target, light, ex)};
            ROOT::RDataFrame df {"SimulationTTree", file};
            auto hRPx {df.Histo1D(HistConfig::RPx, "RPx")};
            hRPx->SetTitle(TString::Format("d = %.2f E_{x} = %.2f", dist, ex));
            hsRPx.back().push_back((TH1D*)hRPx->Clone());
            // SP histogram
            auto f {std::make_unique<TFile>(file.c_str())};
            auto* hSP {f->Get<TH2D>("hSP")};
            hSP->SetDirectory(nullptr);
            hsSP.back().push_back(hSP);
            f->Close();
        }
    }

    // Important: normalize histograms
    for(auto& h : hsAnaRPx)
        h->Scale(1. / h->Integral());
    for(auto& vec : hsRPx)
        for(auto& h : vec)
            h->Scale(1. / h->Integral());

    // Perform minimization
    std::vector<TGraphErrors*> gs;
    for(int i = 0; i < Exs.size(); i++)
    {
        gs.push_back(new TGraphErrors);
        auto& g {gs.back()};
        g->SetTitle(TString::Format("Ex = %.2f MeV;Distance to (0,0) [pads];Chi2", Exs[i]));
        g->SetLineWidth(2);
        g->SetMarkerStyle(24);
        auto& hana {hsAnaRPx[i]};
        for(int j = 0; j < dists.size(); j++)
        {
            auto& dist {dists[j]};
            auto& hsimu {hsRPx[j][i]};
            auto chi2 {GetOverlap(hana, hsimu)};
            g->AddPoint(dist, chi2);
        }
    }

    // Minimize
    for(auto& g : gs)
    {
        TF1 func {"func", [=](double* x, double* p) { return g->Eval(x[0], nullptr, "S"); }, dists.front(),
                  dists.back(), 0};
        auto min {func.GetMinimumX()};
        min = min * 2 - 256;
        auto* text {new TPaveText {0.65, 0.7, 0.85, 0.85, "NDC"}};
        text->SetBorderSize(0);
        text->AddText(TString::Format("%.2f mm", min));
        g->GetListOfFunctions()->Add(text);
    }

    // Draw
    for(int ic = 0; ic < Exs.size(); ic++)
    {
        auto* c {new TCanvas {TString::Format("c%d", ic), TString::Format("Ex = %.2f MeV", Exs[ic])}};
        c->DivideSquare(dists.size() * 1);
        int ip {1};
        for(int id = 0; id < dists.size(); id++)
        {
            c->cd(ip);
            auto* stack {new THStack};
            stack->SetTitle(TString::Format("%s;RP.X() [mm];Normalized counts", hsRPx[id][ic]->GetTitle()));
            stack->Add(hsRPx[id][ic]);
            stack->Add(hsAnaRPx[ic]);
            stack->Draw("nostack histe");
            // hsRPx[id][ic]->DrawNormalized("hist");
            // hsAnaRPx[ic]->DrawNormalized("histe same");
            // ip++;
            // c->cd(ip);
            // hsSP[id][ic]->Draw("colz");
            ip++;
        }
    }

    auto* c0 {new TCanvas {"c-1", "Ex canvas"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hEx->DrawClone();
    for(auto h : hsAnaEx)
        h->Draw("same");
    c0->cd(2);
    hsAnaRPx[0]->Draw("histe");
    c0->cd(3);
    hsAnaRPx[1]->Draw("histe");
    c0->cd(4);
    gs[0]->Draw("apl");
    c0->cd(5);
    gs[1]->Draw("apl");

    auto* list {gROOT->GetListOfCanvases()};
    gSelector->SendToWebsite("sim_to_ana.root", list->FindObject("c0"), "cIterdt0");
    gSelector->SendToWebsite("sim_to_ana.root", list->FindObject("c1"), "cIterdt1");
    gSelector->SendToWebsite("sim_to_ana.root", list->FindObject("c-1"), "cIterdt2");
}
