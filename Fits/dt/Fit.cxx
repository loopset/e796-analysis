#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TFitResult.h"
#include "TGraph.h"
#include "THStack.h"
#include "TROOT.h"
#include "TString.h"

#include "FitData.h"
#include "FitModel.h"
#include "FitPlotter.h"
#include "FitRunner.h"

#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

void Fit()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {
        "Final_Tree", "/media/Data/E796v2/PostAnalysis/RootFiles/Legacy/tree_beam_20O_target_2H_light_3H_front.root"};
    auto gated {df};
    std::cout << "Counts after cuts = " << gated.Count().GetValue() << '\n';

    int nbins {100};
    double hmin {-5};
    double hmax {25};
    auto hEx {gated.Histo1D(
        {"hEx", TString::Format(";E_{x} [MeV];Counts / %.0f keV", (hmax - hmin) / nbins * 1E3), nbins, hmin, hmax},
        "Ex")};
    // Read PS
    ROOT::RDataFrame phase {"simulated_tree", "/media/Data/E796v2/RootFiles/Old/FitJuan/"
                                              "20O_and_2H_to_3H_NumN_1_NumP_0_Ex0_Date_2022_11_29_Time_16_35.root"};
    auto hPS {phase.Histo1D({"hPS", "PS 1n;E_{x} [MeV]", nbins, hmin, hmax}, "Ex_cal")};
    // Format phase space
    hPS->Smooth(20);
    // Scale it
    auto intEx {hEx->Integral()};
    auto intPS {hPS->Integral()};
    double factor {0.15};
    hPS->Scale(factor * intEx / intPS);

    // Data
    double xmin {hmin};
    double xmax {10};
    Fitters::Data data {*hEx, xmin, xmax};

    // Model
    int ngauss {7};
    int nvoigt {0};
    Fitters::Model model {ngauss, nvoigt, {*hPS}};

    // Runner
    Fitters::Runner runner {data, model};
    runner.GetObjective().SetUseDivisions(true);
    // Set init parameters
    double sigma {0.374};
    Fitters::Runner::Init initPars {
        {"g0", {400, 0, sigma}},
        {"g1", {10, 1.5, sigma}},
        {"g2", {110, 3.2, sigma}},
        {"g3", {60, 4.5, sigma}},
        // {"g4", {56, 5.2, sigma}},
        {"g4", {60, 6.7, sigma}},
        {"g5", {65, 7.9, sigma}},
        {"g6", {20, 8.9, sigma}},
        {"ps0", {0.1}},
    };
    // Set bounds and fix parameters
    double minmean {0.3};
    double maxmean {0.3};
    Fitters::Runner::Bounds initBounds {};
    Fitters::Runner::Fixed fixedPars {};
    for(const auto& [key, init] : initPars)
    {
        // determine number of parameters
        auto strkey {TString(key)};
        int npars {};
        if(strkey.Contains("g"))
            npars = 3; // gaussian
        else if(strkey.Contains("v"))
            npars = 4; // voigt
        else if(strkey.Contains("ps"))
            npars = 1;
        else
            throw std::runtime_error("Wrong key received in initPars");
        for(int par = 0; par < npars; par++)
        {
            std::pair<double, double> pair {}; // for bounds
            bool boo {};                       // for fix parameters
            if(par == 0)                       // Amplitude
            {
                pair = {0, 10000};
                boo = false;
            }
            else if(par == 1) // Mean
            {
                pair = {init[par] - minmean, init[par] + maxmean};
                boo = false;
            }
            else if(par == 2) // Sigma
            {
                pair = {-11, -11};
                boo = true; // fix it
            }
            else
                throw std::runtime_error("No automatic config for this parameter index (only gaussians so far)!");
            // Fill
            initBounds[key].push_back(pair);
            fixedPars[key].push_back(boo);
        }
    }
    runner.SetInitial(initPars);
    runner.SetBounds(initBounds);
    runner.SetFixed(fixedPars);
    // Fit!!
    runner.Fit();
    // result
    auto res {runner.GetFitResult()};

    // Save on file
    runner.Write("./Outputs/fit_dt.root");

    // Draw
    Fitters::Plotter plotter {&data, &model, &res};
    auto* gfit {plotter.GetGlobalFit()};
    auto hfits {plotter.GetIndividualHists()};

    // TCanvas
    auto* c0 {new TCanvas {"c0", "(d,t) global fit"}};
    hEx->GetXaxis()->SetRangeUser(xmin, xmax);
    hEx->SetLineWidth(2);
    hEx->DrawClone("e");
    gfit->Draw("same");
    // Stack of histograms
    auto* hs {new THStack};
    for(auto& [key, h] : hfits)
    {
        h->SetLineWidth(2);
        hs->Add(h);
    }
    hs->Draw("plc nostack same");
}
