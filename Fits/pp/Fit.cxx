#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TROOT.h"
#include "TString.h"

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

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "1H")};
    // Ex
    auto hEx {df.Histo1D(E796Fit::Expp, "Ex")};
    // Phase space deuton breakup
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -1, 0)};
    auto hPS {phase.Histo1D(E796Fit::Expp, "Eex", "weight")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr(), 2);

    // Fitting range
    double exmin {-5};
    double exmax {10};

    // Sigmas
    Interpolators::Sigmas sigmas;
    sigmas.Read(gSelector->GetSigmasFile("1H", "1H").Data());

    // Model
    int ngauss {2};
    int nvoigt {0};
    bool cte {false};
    Fitters::Model model {ngauss, nvoigt, {*hPS}, cte};

    // Set init parameters
    double sigma {0.3};
    Fitters::Runner::Init initPars {
        {"g0", {400, 0, sigma}},
        {"g1", {100, 1.67, sigma}},
        {"ps0", {1.5}},
        // {"cte0", {40}},
    };
    // Eval correct sigma
    for(auto& [key, vals] : initPars)
        vals[2] = sigmas.Eval(vals[1]);
    // Set bounds and fix parameters
    double minmean {0.2};
    double maxmean {0.2};
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
        else if(strkey.Contains("ps") || strkey.Contains("cte"))
            npars = 1;
        else
            throw std::runtime_error("Wrong key received in initPars");
        for(int par = 0; par < npars; par++)
        {
            std::pair<double, double> pair {}; // for bounds
            bool boo {};                       // for fix parameters
            if(par == 0)                       // Amplitude
            {
                pair = {0, 100000};
                boo = false;
            }
            else if(par == 1) // Mean
            {
                pair = {init[par] - minmean, init[par] + maxmean};
                boo = false;
            }
            else if(par == 2) // Sigma
            {
                pair = {0.05, 0.25};
                boo = false; // fix it
            }
            else
                throw std::runtime_error("No automatic config for this parameter index (only gaussians so far)!");
            // Fill
            initBounds[key].push_back(pair);
            fixedPars[key].push_back(boo);
        }
    }

    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, initPars, initBounds, fixedPars,
                    ("./Outputs/fit_" + gSelector->GetFlag() + ".root"), "20O(p,p) fit",
                    {{"g0", "g.s"}, {"g1", "1st ex"}, {"ps0", "20O(d,d) breakup"}});
}
