#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"

#include "FitModel.h"
#include "FitRunner.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../Selector/Selector.h"
#include "/media/Data/E796v2/Fits/FitHist.h"
void Fit()
{
    ROOT::EnableImplicitMT();

    // Analysis
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "2H")};
    // Ex
    auto hEx {df.Histo1D(E796Fit::Exdd, "Ex")};
    // Phase space 19O
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, 1, 0)};
    auto hPS {phase.Histo1D(E796Fit::Exdd, "Eex", "weight")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr(), 2);

    // Fitting range
    double exmin {-5};
    double exmax {25};

    // Model
    int ngauss {6};
    int nvoigt {0};
    Fitters::Model model {ngauss, nvoigt, {*hPS}};

    // Sigmas
    Interpolators::Sigmas sigmas;
    sigmas.Read(gSelector->GetSigmasFile("2H", "2H").Data());

    // Set init parameters
    double sigma {0.3};
    Fitters::Runner::Init initPars {{"g0", {400, 0, sigma}},
                                    {"g1", {100, 1.5, sigma}},
                                    {"g2", {50, 4, sigma}},
                                    {"g3", {50, 5.5, sigma}},
                                    {"g4", {50, 7.7, sigma}},
                                    {"g5", {50, 8.5, sigma}},
                                    {"ps0", {1.5}}};
    // Reread in case file exists
    auto outfile {TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().data())};
    if(!gSystem->AccessPathName(outfile))
    {
        std::cout << "Setting parameters from previous fit" << '\n';
        initPars = Fitters::ReadInit(outfile.Data());
    }
    // Eval correct sigma
    for(auto& [key, vals] : initPars)
        vals[2] = sigmas.Eval(vals[1]);
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
                std::cout<<"Par : "<<key<<" mean : "<<init[par]<<'\n';
                pair = {init[par] - minmean, init[par] + maxmean};
                boo = false;
            }
            else if(par == 2) // Sigma
            {
                pair = {0, 0.5};
                boo = true; // fix it
            }
            else
                throw std::runtime_error("No automatic config for this parameter index (only gaussians so far)!");
            // Fill
            initBounds[key].push_back(pair);
            fixedPars[key].push_back(boo);
        }
    }

    // Run for all the nodes
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, initPars, initBounds, fixedPars,
                    ("./Outputs/fit_" + gSelector->GetFlag() + ".root"), "20O(d,d) fit",
                    {{"g0", "g.s"}, {"g1", "1st ex"}});
}
