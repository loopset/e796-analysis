#include "TFile.h"
#include "TString.h"

#include "FitInterface.h"
#include "ModelPlotter.h"
#include "PhysColors.h"
#include "PhysSF.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "../../Selector/Selector.h"

void models()
{
    double ymin {0};  // MeV
    double ymax {18}; // MeV
    int nmodels {2};

    // Parse
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root", "./Outputs/fit_juan_RPx.root");
    auto file {std::make_unique<TFile>("./Outputs/sfs.root")};
    std::vector<double> Exs;
    std::vector<double> gammas;
    std::vector<std::string> sfs;
    for(const auto peak : inter.GetPeaks())
    {
        auto sfcol {file->Get<PhysUtils::SFCollection>((peak + "_sfs").c_str())};
        if(!sfcol)
            continue;
        auto ex {inter.GetParameter(peak, 1)};
        Exs.push_back(ex);
        auto gamma {inter.GetParameter(peak, 3)};
        if(gamma == -11 || gamma <= 0.01)
            gammas.push_back(0);
        else
            gammas.push_back(gamma);
        auto sf {sfcol->GetBestChi2()};
        if(sf)
            sfs.push_back(PlotUtils::ModelToPlot::FormatSF(
                sf->GetSF(), sf->GetUSF())); // sfs.push_back(TString::Format("%.2f", sf->GetSF()).Data());
        else
            sfs.push_back("");
    }
    file->Close();

    // Define models to plot
    PlotUtils::ModelToPlot ours {"Our work"};
    ours.SetEx(Exs);
    ours.SetGammas(gammas);
    ours.SetSF(sfs);
    ours.SetUniqueColor(gPhysColors->Get(14));

    PlotUtils::ModelToPlot msta {"YSOX"};
    // Parse theoretical file
    double be {7.655};
    std::vector<std::pair<double, double>> theo {
        {{7.655, 3.45}, {13.558, 0.101}, {9.016, 0.229}, {12.738, 0.487}, {13.991, 0.583}}};
    // Transform
    std::for_each(theo.begin(), theo.end(), [&](auto& p) { p.first = p.first - be; });
    std::sort(theo.begin(), theo.end(), [](auto& a, auto& b) { return a.first < b.first; });
    std::vector<double> theoEx {};
    std::vector<std::string> theosfs {};
    for(const auto& [ex, sf] : theo)
    {
        theoEx.push_back(ex);
        theosfs.push_back(TString::Format("%.2f", sf).Data());
    }
    msta.SetEx(theoEx);
    msta.SetSF(theosfs);
    msta.SetUniqueColor(gPhysColors->Get(5));

    PlotUtils::ModelPlotter mpl {ymin, ymax, nmodels};
    mpl.SetYaxisLabel("E_{x} [MeV]");
    mpl.AddModel(ours);
    mpl.AddModel(msta);

    auto canv = mpl.Draw();

    gSelector->SendToWebsite("dt.root", canv, "cYSOX");
}
