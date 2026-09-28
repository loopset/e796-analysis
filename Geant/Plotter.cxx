#ifndef Plotter_cxx
#define Plotter_cxx

#include "ActColors.h"
#include "ActKinematics.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "THStack.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TVirtualPad.h"

#include <string>
#include <vector>

#include "/media/Data/E796v2/Geant/Analyser.cxx"

void Fit(TH1* h, TGraphErrors* g, double gamma)
{
    auto binOfMax {h->GetMaximumBin()};
    auto yOfMax {h->GetBinContent(binOfMax)};
    auto xOfMax {h->GetBinCenter(binOfMax)};
    TF1* f {};
    double width {};
    if(gamma == 0)
    {
        f = new TF1 {"f", "gaus", -5, 20};
        f->SetParameters(10, xOfMax, 0.1);
        f->SetParNames("Amp", "Mean", "Sigma");
        f->SetParLimits(0, 0, 1e8);
        f->SetParLimits(2, 0, 2);
        width = 0.5;
    }
    else
    {
        f = new TF1 {"f", "[0] * TMath::Voigt(x - [1], [2], [3])", -10, 40};
        f->SetParameters(yOfMax, xOfMax, 0.1, gamma);
        f->SetParNames("Amp", "Mean", "Sigma", "Gamma");
        f->SetParLimits(0, 0, 1e8);
        f->SetParLimits(2, 0, 2);
        f->FixParameter(3, gamma);
        width = 15;
    }
    h->Fit(f, "0R+", "", xOfMax - width, xOfMax + width);
    f->ResetBit(TF1::kNotDraw);

    // And push fit results to graph
    auto mu {f->GetParameter(1)};
    auto umu {f->GetParError(1)};
    auto sigma {f->GetParameter(2)};
    auto usigma {f->GetParError(2)};
    g->AddPointError(mu, sigma, umu, usigma);
    std::cout << "Fit results : " << f->GetParameter(1) << " +/- " << f->GetParError(1) << " and " << f->GetParameter(2)
              << " +/- " << f->GetParError(2) << '\n';
}

class RetPlot
{
public:
    TMultiGraph* mEffs {};
    TH2D* hKins {};
    TH1D* hExs {};
    TGraphErrors* gsigmas {};
    TH2D* hPID {};
};

RetPlot Plotter(const std::string& beam, const std::string& target, const std::string& light, double ebeam,
                const std::vector<double>& exs, const std::vector<double>& gammas)
{
    std::vector<RetAna> rets;
    auto* gsigmas {new TGraphErrors};
    gsigmas->SetTitle("#sigma with E_{x};E_{x} [MeV];#sigma [MeV]");
    int idx {};
    for(const auto& ex : exs)
    {
        auto file {GetAnaFile(beam, target, light, ebeam, ex)};
        auto* f {new TFile {file}};
        // Rebuild RetAna
        RetAna ret {.hKinSampled = f->Get<TH2D>("hKinSampled"),
                    .hKin = f->Get<TH2D>("hKin"),
                    .hPID = f->Get<TH2D>("hPID"),
                    .hCMAll = f->Get<TH1D>("hCMAll"),
                    .hCMAfter = f->Get<TH1D>("hCMAfter"),
                    .hEx = f->Get<TH1D>("hEx"),
                    .eff = f->Get<TEfficiency>("eff"),
                    .hEStragg = f->Get<TH2D>("hEStragg")};
        rets.push_back(ret);
        // Fit
        Fit(ret.hEx, gsigmas, gammas[idx]);
        idx++;
    }

    // Create copies to add
    auto hKin {(TH2D*)rets[0].hKin->Clone()};
    hKin->Reset();
    auto hPID {(TH2D*)rets[0].hPID->Clone()};
    hPID->Reset();
    auto hEx {(TH1D*)rets[0].hEx->Clone()};
    hEx->Reset();
    auto hEStragg {(TH2D*)rets[0].hEStragg->Clone()};
    hEStragg->Reset();
    auto* mEffs {new TMultiGraph};

    // Histo stack
    auto* stack {new THStack};

    for(auto& ret : rets)
    {
        hKin->Add(ret.hKin);
        hPID->Add(ret.hPID);
        hEx->Add(ret.hEx);
        stack->Add(ret.hEx);
        mEffs->Add(ret.eff->CreateGraph());
        hEStragg->Add(ret.hEStragg);
    }

    // Plot
    static int cPlotIdx {0};
    auto* c0 {new TCanvas {TString::Format("cPlot%d", cPlotIdx), "Simu plotter canvas"}};
    cPlotIdx++;
    c0->DivideSquare(6);
    c0->cd(1);
    hKin->Draw("colz");
    for(int i = 0; i < exs.size(); i++)
    {
        auto ex {exs[i]};
        ActPhysics::Kinematics k {
            TString::Format("%s(%s,%s)@%.2f|%.2f", beam.c_str(), target.c_str(), light.c_str(), ebeam, ex).Data()};
        auto* g {k.GetKinematicLine3()};
        g->SetLineColor(i + 2);
        g->Draw("l");
    }
    c0->cd(2);
    hEx->SetLineWidth(2);
    hEx->Draw();
    stack->Draw("nostack plc pmc same");
    // for(int i = 0; i < rets.size(); i++)
    // {
    //     auto& h {rets[i].hEx};
    //     h->SetLineColor(i + 2);
    //     h->Draw("same");
    // }
    c0->cd(3);
    mEffs->Draw("apl plc pmc");
    gPad->BuildLegend();
    c0->cd(4);
    hPID->Draw("colz");
    c0->cd(5);
    gsigmas->SetMarkerStyle(24);
    gsigmas->Draw("apl");
    c0->cd(6);
    hEStragg->Draw("colz");

    // Build ret
    RetPlot ret {.mEffs = mEffs, .hKins = hKin, .hExs = hEx, .gsigmas = gsigmas, .hPID = hPID};
    return ret;
}

#endif
