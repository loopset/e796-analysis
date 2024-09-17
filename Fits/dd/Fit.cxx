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
#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"

void Fit()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Final_Tree", E796Utils::GetFileName(2, "20O", "2H", "2H", true)};

    // Nodes at which compute global fit
    std::vector<ROOT::RDF::RNode> nodes {df, df.Filter(E796Gates::rpx1<>, {"fRP"}),
                                         df.Filter(E796Gates::rpx2<>, {"fRP"})};
    std::vector<std::string> labels {"dt", "dt_1", "dt_2"};
    std::vector<TH1D*> hExs;
    for(int i = 0; i < nodes.size(); i++)
    {
        auto h {nodes[i].Histo1D(E796Fit::Exdd, "Ex")};
        hExs.push_back((TH1D*)h->Clone());
    }
    // Fitting range
    double exmin {-5};
    double exmax {10};

    // Model
    int ngauss {2};
    int nvoigt {0};
    Fitters::Model model {ngauss, nvoigt, {}};

    // Set init parameters
    double sigma {0.3};
    Fitters::Runner::Init initPars {
        {"g0", {400, 0, sigma}}, {"g1", {100, 1.5, sigma}},
        // {"g2", {110, 3.2, sigma}}, {"g3", {60, 4.5, sigma}},
        // {"g4", {60, 6.7, sigma}}, {"g5", {65, 7.9, sigma}}, {"g6", {20, 8.9, sigma}},
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
                boo = false; // fix it
            }
            else
                throw std::runtime_error("No automatic config for this parameter index (only gaussians so far)!");
            // Fill
            initBounds[key].push_back(pair);
            fixedPars[key].push_back(boo);
        }
    }

    // Run for all the nodes
    for(int i = 0; i < hExs.size(); i++)
    {
        Fitters::RunFit(hExs[i], exmin, exmax, model, initPars, initBounds, fixedPars,
                        ("./Outputs/fit_" + labels[i] + ".root"), labels[i], {{"g0", "g.s"}, {"g1", "1st ex"}});
    }
}
