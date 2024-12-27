#include "ActSilMatrix.h"

#include "TFile.h"
#include "TString.h"

#include <iostream>

#include "./GetContourFuncs.cxx"
void DoFits(TString mode)
{
    mode.ToLower();
    std::cout << "Running for : " << mode << '\n';
    // Assign things depending on mode
    bool isSide {};
    std::vector<int> idxs;
    if(mode == "side")
    {
        isSide = true;
        idxs = {0, 1, 2, 3, 4, 5, 6, 7};
    }
    else if(mode == "antiveto")
        idxs = {0, 2, 3, 4, 5, 7, 8, 10};
    else if(mode == "veto")
        idxs = {0, 2, 3, 4, 5, 7, 8, 10};
    else
        throw std::invalid_argument("DoHists: no known mode " + mode);

    // Read data
    auto fin {std::make_unique<TFile>(TString::Format("./Inputs/%s_histograms.root", mode.Data()).Data())};
    // Set histograms to process
    std::map<int, TH1D*> pxys, pzs;
    for(auto& idx : idxs)
    {
        auto xykey {TString::Format("pxy%d", idx)};
        auto zkey {TString::Format("pz%d", idx)};
        pxys[idx] = fin->Get<TH1D>(xykey);
        pxys[idx]->SetDirectory(nullptr);
        pzs[idx] = fin->Get<TH1D>(zkey);
        pzs[idx]->SetDirectory(nullptr);
    }

    ProjMap nxys, nzs;
    for(const auto& idx : idxs)
    {
        double w {5};
        double s {0.2};
        // XY
        auto* fx {FindBestFit(pxys[idx], w, s)};
        nxys[idx] = ScaleWithFunc(pxys[idx], fx);
        // Z
        auto* fz {FindBestFit(pzs[idx], w, s)};
        nzs[idx] = ScaleWithFunc(pzs[idx], fz);
    }


    // Fit to contour
    double thresh {0.65};
    double width {15};
    // XY
    auto xypoints {FitToCountour(nxys, thresh, width)};
    // Z
    auto zpoints {FitToCountour(nzs, thresh, width)};


    // Build class to store them
    auto* smatrix {new ActPhysics::SilMatrix {mode.Data()}};
    for(auto& [i, ypoint] : xypoints)
    {
        const auto& zpoint {zpoints[i]};
        smatrix->AddSil(i, ypoint, zpoint);
    }

    // plot
    auto* cpx {new TCanvas("cpx", "XY projection canvas")};
    PlotAll(cpx, pxys);

    auto* cnx {new TCanvas("cnx", "Normalized XY canvas")};
    PlotAll(cnx, nxys);

    auto* cpz {new TCanvas("cpz", "Z projection canvas")};
    PlotAll(cpz, pzs);

    auto* cnz {new TCanvas("cnz", "Normalized Z canvas")};
    PlotAll(cnz, nzs);


    auto* cm {new TCanvas("cm", "Sil matrix")};
    auto name {TString::Format("./Outputs/%s_matrix.root", mode.Data())};
    std::cout << "Saving matrix at : " << name << '\n';
    smatrix->Write(name.Data());
    smatrix->Draw(false);
}
