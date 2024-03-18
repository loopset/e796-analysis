#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TROOT.h"
#include "TString.h"

#include "FitModel.h"
#include "FitRunner.h"
#include "FitUtils.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "/media/Data/E796v2/Fits/FitHist.h"
#include "/media/Data/E796v2/PostAnalysis/Gates.cxx"

void Fit()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {
        "Final_Tree", "/media/Data/E796v2/PostAnalysis/RootFiles/Legacy/tree_beam_20O_target_1H_light_2H_front.root"};

    // Different cuts!
    std::vector<ROOT::RDF::RNode> nodes {df, df.Filter(E796Gates::rpx1, {"fRP"}), df.Filter(E796Gates::rpx2, {"fRP"})};
    std::vector<std::string> labels {"pd", "pd_1", "pd_2"};
    std::vector<TH1D*> hExs;
    for(int i = 0; i < nodes.size(); i++)
    {
        auto h {nodes[i].Histo1D(E796Fit::Expd, "Ex")};
        hExs.push_back((TH1D*)h->Clone());
    }

    // Fitting range
    double exmin {-10};
    double exmax {5};

    // Model
    int ngauss {3};
    int nvoigt {0};
    bool cte {true};
    Fitters::Model model {ngauss, nvoigt, {}, cte};

    // Set init parameters
    double sigma {0.240};
    Fitters::Runner::Init initPars {
        {"g0", {2000, 0, sigma}},
        {"g1", {250, 1.5, sigma}},
        {"g2", {110, 3.2, sigma}},
        {"cte0", {4}},
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

    // Run for all the cuts
    for(int i = 0; i < hExs.size(); i++)
    {
        Fitters::RunFit(hExs[i], exmin, exmax, model, initPars, initBounds, fixedPars,
                        ("./Outputs/fit_" + labels[i] + ".root"), labels[i]);
    }
}
