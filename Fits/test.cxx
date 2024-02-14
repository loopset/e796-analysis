#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TFitResult.h"
#include "TGraph.h"
#include "TROOT.h"
#include "TRatioPlot.h"
#include "TSpline.h"

#include "Fit/BinData.h"
#include "Fit/DataOptions.h"
#include "Fit/DataRange.h"

#include "HFitInterface.h"

#include "/media/Data/PhysicsClasses/src/FitModel.cxx"
#include "/media/Data/PhysicsClasses/src/FitRunner.cxx"
#include "/media/Data/PhysicsClasses/src/Fitter.cxx"

void test()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {
        "Final_Tree", "/media/Data/E796v2/PostAnalysis/RootFiles/Legacy/tree_beam_20O_target_2H_light_3H_front.root"};
    int nbins {100};
    double hmin {-5};
    double hmax {25};
    auto hEx {df.Histo1D(
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
    // Convert to TSpline3
    auto* spe {new TSpline3 {hPS.GetPtr(), "b2,e2", 0, 0}};

    // Data
    double xmin {hmin};
    double xmax {10};
    ROOT::Fit::DataOptions opts;
    opts.fIntegral = false;
    ROOT::Fit::DataRange range {xmin, xmax};
    ROOT::Fit::BinData data {opts, range};
    ROOT::Fit::FillData(data, hEx.GetPtr());

    // Model
    int ng {7};
    int nv {0};
    bool cte {false};
    auto* model = new Fitters::Model {ng, nv, {spe}, cte};
    auto [f1, wrap] {model->Wrap(xmin, xmax)};

    // Runner
    Fitters::Runner runner {model};
    runner.SetModelWrap(wrap);
    // Set parameters
    double sigma {0.374};
    // Init parameters
    Fitters::Runner::Init init {
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
    double lowMean {0.3};
    double upMean {0.3};
    Fitters::Runner::Bounds bounds {};
    Fitters::Runner::Fixed fixed {};
    for(const auto& [key, vals] : init)
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
                pair = {vals[par] - lowMean, vals[par] + upMean};
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
            bounds[key].push_back(pair);
            fixed[key].push_back(boo);
        }
    }
    runner.SetInitial(init);
    runner.SetBounds(bounds);
    runner.SetFixed(fixed);
    runner.Fit(data);
    auto res {runner.GetFitResult()};

    auto* g {new TGraph};
    res.Scan(1, g);

    auto* hclone {(TH1D*)hEx->Clone()};
    hclone->GetListOfFunctions()->Add(f1);

    // Ratio plot
    auto* c0 {new TCanvas {"c0", "test"}};
    auto* ratio {new TRatioPlot {hclone, "", &res}};
    ratio->Draw();

    auto* c1 {new TCanvas};
    g->Draw("apl");

}
