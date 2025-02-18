#include "TFile.h"
#include "TString.h"
#include "TStyle.h"

#include "FitInterface.h"
#include "ModelPlotter.h"
#include "PhysColors.h"
#include "PhysSF.h"
#include "PhysSM.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "../../Selector/Selector.h"

void models()
{
    gStyle->SetTextFont(132);

    double ymin {0};  // MeV
    double ymax {18}; // MeV
    int nmodels {3};

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
    ours.SetJp({"5/2+", "1/2+", "(1/2,3/2)-", "(1/2,3/2)-", "(1/2,3/2)-", "1/2+", "(1/2,3/2-)", "(1/2,3/2)-"});
    ours.SetUniqueColor(gPhysColors->Get(14));

    PhysUtils::ModelParser ysox {{"./Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
                                  "./Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt"}};
    ysox.ShiftEx();
    ysox.MaskExAbove(10);
    ysox.MaskSFBelow(0.1);

    PlotUtils::ModelToPlot mysox {"YSOX"};
    mysox.SetFromParser(&ysox);
    // // Parse theoretical file
    // double be {7.655};
    // std::vector<std::tuple<double, double, std::string>> theo {{{7.655, 3.45, "5/2+"},
    //                                                             {13.558, 0.101, "3/2+"},
    //                                                             {9.016, 0.229, "1/2+"},
    //                                                             {12.738, 0.487, "1/2-"},
    //                                                             {13.991, 0.583, "1/2-"}}};
    // // Transform
    // std::for_each(theo.begin(), theo.end(), [&](auto& p) { std::get<0>(p) = std::get<0>(p) - be; });
    // std::sort(theo.begin(), theo.end(), [](auto& a, auto& b) { return std::get<0>(a) < std::get<0>(b); });
    // std::vector<double> theoEx {};
    // std::vector<std::string> theosfs, theojpi;
    // for(const auto& [ex, sf, jpi] : theo)
    // {
    //     theoEx.push_back(ex);
    //     theosfs.push_back(TString::Format("%.2f", sf).Data());
    //     theojpi.push_back(jpi);
    // }
    // mysox.SetEx(theoEx);
    // mysox.SetSF(theosfs);
    // mysox.SetJp(theojpi);
    mysox.SetUniqueColor(gPhysColors->Get(5));

    // Ramus thesis
    PlotUtils::ModelToPlot mramus {"A. Ramus"};
    mramus.SetEx({0, 1.459, 3.23});
    mramus.SetSF({"4.70(94)", "0.50(11)", "1.47(30)"});
    mramus.SetJp({"5/2+", "1/2+", "(1/2,3/2)-"});


    PlotUtils::ModelPlotter mpl {ymin, ymax, nmodels};
    mpl.SetYaxisLabel("E_{x} [MeV]");
    mpl.AddModel(ours);
    mpl.AddModel(mysox);
    mpl.AddModel(mramus);

    auto canv = mpl.Draw();

    gSelector->SendToWebsite("dt.root", canv, "cYSOX");
}
