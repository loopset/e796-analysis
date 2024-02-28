#ifndef FitUtils_cxx
#define FitUtils_cxx

#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "THStack.h"
#include "TString.h"

#include "FitData.h"
#include "FitModel.h"
#include "FitPlotter.h"
#include "FitRunner.h"
#include "PhysColors.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>

namespace E796Fit
{
void SaveHisto(TH1D* h, const std::string& file, const std::string& name)
{
    auto f {std::make_unique<TFile>(file.c_str(), "recreate")};
    f->cd();
    h->Write(name.c_str());
}

TH1D* ReadHisto(const std::string& file, const std::string& name)
{
    auto f {std::make_unique<TFile>(file.c_str())};
    auto* h {f->Get<TH1D>(name.c_str())};
    if(!h)
        throw std::runtime_error("E796Fit::ReadHisto(): could not read histo " + name);
    h->SetDirectory(nullptr);
    return h;
}

std::tuple<int, double, double> GetHistoSpecs(TH1D* h)
{
    return {h->GetNbinsX(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax()};
}

void DrawGlobalFit(TGraph* g, const std::unordered_map<std::string, TH1D*>& hs)
{
    // Draw in current gPad
    g->Draw("same");
    auto* stack {new THStack};
    for(auto& [key, h] : hs)
    {
        h->SetLineWidth(2);
        stack->Add(h);
    }
    stack->Draw("nostack plc same");
}

void RunFit(TH1D* h, double exmin, double exmax, Fitters::Model& model, const Fitters::Runner::Init& initial,
            const Fitters::Runner::Bounds& bounds, const Fitters::Runner::Fixed& fixed, const std::string& outfile,
            const std::string& title)
{
    std::cout << BOLDCYAN << "++++ Global fit " << title << " ++++" << RESET << '\n';
    // Init data
    Fitters::Data data {*h, exmin, exmax};

    // Init runner
    Fitters::Runner runner {data, model};
    runner.GetObjective().SetUseDivisions(true);
    // And initial parameters
    runner.SetInitial(initial);
    runner.SetBounds(bounds);
    runner.SetFixed(fixed);
    // Run
    runner.Fit();
    // Save
    runner.Write(outfile);

    // Get fit result
    auto res {runner.GetFitResult()};

    // Plotter
    Fitters::Plotter plot {&data, &model, &res};
    auto* gfit {plot.GetGlobalFit()};
    auto hfits {plot.GetIndividualHists()};

    // Draw
    static int cCount {};
    auto* c {new TCanvas {TString::Format("cGF%d", cCount), title.c_str()}};
    cCount++;
    h->GetXaxis()->SetRangeUser(exmin, exmax);
    h->SetLineWidth(2);
    h->DrawClone("e");
    DrawGlobalFit(gfit, hfits);
    // End :)
    std::cout << BOLDCYAN << "++++++++++++++++++++++++++++++" << RESET << '\n';
}

} // namespace E796Fit

#endif
