#include "TFile.h"
#include "TString.h"
#include "TStyle.h"

#include "FitInterface.h"
#include "ModelPlotter.h"
#include "PhysColors.h"
#include "PhysSF.h"
#include "PhysSM.h"

#include <string>
#include <vector>

#include "../../Selector/Selector.h"

void models()
{
    gStyle->SetTextFont(132);
    gStyle->SetTextSize(0.025);

    double ymin {0};  // MeV
    double ymax {20}; // MeV
    int nmodels {4};

    // Parse
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root", "./Outputs/fit_juan_RPx.root");
    auto file {std::make_unique<TFile>("./Outputs/sfs.root")};
    std::map<std::string, PhysUtils::SMData> exp;
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
        exp[peak] = PhysUtils::SMData(ex, sf->GetSF(), gamma);
        // Add uncertainty in sf
        exp[peak].SetuSF(sf->GetUSF());
    }
    file->Close();

    // Define models to plot
    PlotUtils::ModelToPlot ours {"Our work"};
    ours.SetEx(Exs);
    ours.SetGammas(gammas);
    ours.SetSF(sfs);
    ours.SetJp({"5/2+", "1/2+", "(1/2,3/2)-", "(1/2,3/2)-", "(1/2,3/2)-", "1/2+", "(1/2,3/2-)", "(1/2,3/2)-"});
    ours.SetUniqueColor(gPhysColors->Get(14));

    // Paramerers to plot theo models
    double maxEx {18};
    double minSF {0.075};

    // YSOX
    PhysUtils::SMParser ysox {
        {"./Inputs/SM/log_O20_O19_ysox_tr_j0p_m1p.txt", "./Inputs/SM/log_O20_O19_ysox_tr_j0p_m1n.txt"}};
    ysox.ShiftEx();
    ysox.MaskExAbove(maxEx);
    ysox.MaskSFBelow(minSF);
    PlotUtils::ModelToPlot mysox {"YSOX"};
    mysox.SetFromParser(&ysox);
    mysox.SetUniqueColor(gPhysColors->Get(5));

    // SFO-tls
    PhysUtils::SMParser sfotls {{"./Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1p.txt",
                                 "./Inputs/SM/log_O20_O19_psdmk2_sfotls_tr_j0p_m1n.txt"}};
    sfotls.ShiftEx();
    sfotls.MaskExAbove(maxEx);
    sfotls.MaskSFBelow(minSF);
    PlotUtils::ModelToPlot msfotls {"SFO-tls"};
    msfotls.SetFromParser(&sfotls);
    msfotls.SetUniqueColor(gPhysColors->Get(14));

    // Ramus thesis
    PlotUtils::ModelToPlot mramus {"A. Ramus"};
    mramus.SetEx({0, 1.459, 3.23});
    mramus.SetSF({"4.70(94)", "0.50(11)", "1.47(30)"});
    mramus.SetJp({"5/2+", "1/2+", "(1/2,3/2)-"});


    PlotUtils::ModelPlotter mpl {ymin, ymax, nmodels};
    mpl.SetYaxisLabel("E_{x} [MeV]");
    mpl.AddModel(ours);
    mpl.AddModel(mramus);
    mpl.AddModel(mysox);
    mpl.AddModel(msfotls);

    auto canv = mpl.Draw();

    gSelector->SendToWebsite("dt.root", canv, "cYSOX");
}
