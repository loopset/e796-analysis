#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TLine.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"
#include "TVirtualPad.h"

#include "FitModel.h"
#include "FitRunner.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "/media/Data/E796v2/Fits/FitHist.h"
#include "/media/Data/E796v2/Selector/Selector.h"

void Fit()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")};

    // Nodes at which compute global fit
    std::vector<ROOT::RDF::RNode> nodes {df};
    std::vector<std::string> labels {gSelector->GetFlag()};
    std::vector<TH1D*> hExs;
    for(int i = 0; i < nodes.size(); i++)
    {
        auto h {nodes[i].Histo1D(E796Fit::Exdt, "Ex")};
        hExs.push_back((TH1D*)h->Clone());
    }
    // Read PS
    // ROOT::RDataFrame phase {"simulated_tree", "/media/Data/E796v2/RootFiles/Old/FitJuan/"
    //                                           "20O_and_2H_to_3H_NumN_1_NumP_0_Ex0_Date_2022_11_29_Time_16_35.root"};
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile(0, 1, 0)};
    // auto hPS {phase.Histo1D(E796Fit::Exdt, "Ex_cal", "Weight_sim")};
    auto hPS {phase.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    hPS->SetNameTitle("hPS", "1n PS");
    Fitters::TreatPS(hExs.front(), hPS.GetPtr());

    // Sigma interpolators
    Interpolators::Sigmas sigmas {gSelector->GetSigmasFile().Data()};

    // Fitting range
    double exmin {-5};
    double exmax {25};

    // Model
    int ngauss {3};
    int nvoigt {8};
    Fitters::Model model {ngauss, nvoigt, {*hPS}};

    // Set init parameters
    // double sigma {0.374};
    double sigma {0.364};
    Fitters::Runner::Init initPars {
        {"g0", {400, 0, sigma}},       {"g1", {10, 1.5, sigma}},       {"g2", {110, 3.2, sigma}},
        {"v0", {60, 4.5, sigma, 0.1}}, {"v1", {60, 6.7, sigma, 0.1}},  {"v2", {65, 7.9, sigma, 0.1}},
        {"v3", {20, 11, sigma, 0.1}},  {"v4", {5, 12.8, sigma, 0.1}},  {"v5", {20, 14.9, sigma, 0.1}},
        {"v6", {10, 16, sigma, 0.1}},  {"v7", {10, 17.5, sigma, 0.1}}, {"ps0", {0.1}},
    };
    // Reread in case file exists
    auto outfile {TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().data())};
    if(!gSystem->AccessPathName(outfile))
    {
        initPars = Fitters::ReadInit(outfile.Data());
    }
    // Eval correct sigma
    for(auto& [key, vals] : initPars)
        vals[2] = sigmas.Eval(vals[1]);

    // Set bounds and fix parameters
    double minmean {0.5};
    double maxmean {0.5};
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
            else if(par == 3)
            {
                pair = {0, 10};
                boo = false;
            }
            else
                throw std::runtime_error("No automatic config for this parameter index");
            // Fill
            initBounds[key].push_back(pair);
            fixedPars[key].push_back(boo);
        }
    }

    // Run for all the nodes
    for(int i = 0; i < hExs.size(); i++)
    {
        Fitters::RunFit(hExs[i], exmin, exmax, model, initPars, initBounds, fixedPars,
                        ("./Outputs/fit_" + labels[i] + ".root"), labels[i]);
    }
    gPad->Update();
    gPad->cd();
    auto* line {new TLine {3.956, gPad->GetUymin(), 3.956, gPad->GetUymax()}};
    line->SetLineWidth(2);
    line->SetLineColor(kMagenta);
    line->Draw("same");
}
