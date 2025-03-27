#include "Rtypes.h"
#include <regex>

#include "TAxis.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TMath.h"
#include "TPaveText.h"
#include "TRandom.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "AngComparator.h"
#include "uncertainties.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../../Selector/Selector.h"

namespace fs = std::filesystem;

std::map<double, std::string> GetFiles(const std::string& dir)
{
    std::map<double, std::string> ret;
    std::regex regex {R"(^beta_([0-9]+\.[0-9]+)$)"};
    // Check if dir exists
    if(!fs::exists(dir))
    {
        std::cout << "Dir : " << dir << " does not exists, skipping!" << '\n';
        return {};
    }
    for(const auto& entry : fs::directory_iterator(dir))
    {
        const auto& path {entry.path()};
        // Check if it is a directory and matches the regex
        if(fs::is_directory(entry))
        {
            std::smatch match;
            std::string name {path.filename().string()};
            if(std::regex_match(name, match, regex))
            {
                auto val {std::stod(match[1].str())};
                ret[val] = name;
            }
        }
    }
    return ret;
}
struct BetaSearch
{
    TH1D* fHist {};
    unc::udouble fBeta {};
};

BetaSearch FindBeta(TGraphErrors* g)
{
    BetaSearch ret {};
    ret.fHist = new TH1D {"hBeta", "Beta estimation", 200, 0, 2};
    int niter {10000};
    int npoints {g->GetN()};
    auto xmin {TMath::MinElement(g->GetN(), g->GetX())};
    auto xmax {TMath::MaxElement(g->GetN(), g->GetX())};
    for(int i = 0; i < niter; i++)
    {
        TGraphErrors giter {};
        for(int p = 0; p < npoints; p++)
        {
            auto x {g->GetPointX(p)};
            auto y {g->GetPointY(p)};
            auto uy {g->GetErrorY(p)};
            giter.SetPoint(p, x, gRandom->Gaus(y, uy));
        }
        // Find zero
        TF1 func {"func", [&](double* x, double* p) { return giter.Eval(x[0]); }, xmin, xmax, 0};
        auto root {func.GetX(1, xmin, xmax)};
        ret.fHist->Fill(root);
    }
    // Get results
    ret.fBeta = {ret.fHist->GetMean(), ret.fHist->GetStdDev()};
    return ret;
}

void plot()
{
    // Set states
    std::vector<std::string> states {"g1_Khan", "g1_BG"};
    std::vector<Angular::Comparator> comps;
    std::vector<TGraphErrors*> gexps;
    std::vector<TGraphErrors*> gs;

    // Open file with experimental xs
    auto* f {new TFile {"../Outputs/xs.root"}};

    // Init canvas
    auto* c0 {new TCanvas {"cBeta2", "BetaK search for pp"}};
    c0->DivideSquare(states.size() * 2);

    // Do!
    int pad {1};
    for(const auto& state : states)
    {
        TString tstr {state.c_str()};
        tstr.ToLower();
        TGraphErrors* gexp {};
        if(tstr.Contains("khan"))
        {
            gexp = new TGraphErrors {"./../Reanalysis/inelastic.dat", "%lg %lg"};
        }
        else
        {
            auto it {state.find_first_of("_")};
            auto key {state.substr(0, it)};
            gexp = f->Get<TGraphErrors>(("g" + key).c_str());
            // gexp->Scale(2.633);
        }
        if(!gexp)
        {
            f->ls();
            throw std::runtime_error("Cannot open TGraphErrors for " + state);
        }
        gexps.push_back(gexp);
        comps.emplace_back(state, gexp);
        auto& comp {comps.back()};
        // Read data
        auto dirs {GetFiles(state)};
        // Add to comparator
        for(const auto& [beta, dir] : dirs)
        {
            std::cout << "beta2 : " << beta << '\n';
            std::string str {TString::Format("#beta = %.2f", beta).Data()};
            comp.Add(str, state + "/" + dir + "/fort.202");
        }
        comp.Fit();
        comp.Draw("", false, true, 3, c0->cd(pad));
        gPad->GetListOfPrimitives()->RemoveLast();
        // And add curves
        gs.push_back(new TGraphErrors);
        auto& g {gs.back()};
        g->SetTitle((state + ";#beta_{2};Scaling factor").c_str());
        for(const auto& [beta, dir] : dirs)
        {
            std::string str {TString::Format("#beta = %.2f", beta).Data()};
            auto sf {comp.GetSF(str)};
            auto usf {comp.GetuSF(str)};
            g->AddPoint(beta, sf);
            g->SetPointError(g->GetN() - 1, 0, usf);
        }
        pad++;
    }

    std::vector<BetaSearch> res;
    // Find 1 == crossing point
    for(auto& g : gs)
    {
        auto r {FindBeta(g)};
        res.push_back(r);

        // auto xmin {TMath::MinElement(g->GetN(), g->GetX())};
        // auto xmax {TMath::MaxElement(g->GetN(), g->GetY())};
        // // Build TF1
        // TF1 func {"func", [&](double* x, double* p) { return g->Eval(x[0], nullptr, "S"); }, xmin, xmax, 0};
        // auto root {func.GetX(1, xmin, xmax)};
        auto* text {new TPaveText {0.55, 0.7, 0.85, 0.85, "NDC"}};
        text->SetBorderSize(0);
        text->AddText(TString::Format("#beta_{L} = %.4f #pm %.4f", r.fBeta.n(), r.fBeta.s()));
        g->GetListOfFunctions()->Add(text);
    }

    // Draw graphs
    pad = states.size() + 1;
    for(int i = 0; i < states.size(); i++)
    {
        c0->cd(pad);
        auto& g {gs[i]};
        g->SetLineWidth(2);
        g->SetLineColor(8);
        g->SetMarkerStyle(24);
        g->SetMarkerColor(8);
        g->Draw("apl");
        g->GetXaxis()->UnZoom();
        pad++;
    }

    gSelector->SendToWebsite("dd.root", c0, "cBeta2");
}
