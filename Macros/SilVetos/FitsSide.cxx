#include "ActSilMatrix.h"

#include "TFile.h"

#include <map>
#include <memory>

#include "./GetContourFuncs.cxx"

void FitsSide()
{
    // Read data
    auto fin {std::make_unique<TFile>("./Inputs/side_histograms.root")};
    // Set histograms to process
    std::vector<int> idxs {0, 1, 2, 3, 4, 5, 6, 7};
    std::map<int, TH1D*> pxs, pzs;
    for(auto& idx : idxs)
    {
        auto xkey {TString::Format("px%d", idx)};
        auto zkey {TString::Format("pz%d", idx)};
        pxs[idx] = fin->Get<TH1D>(xkey);
        pxs[idx]->SetDirectory(nullptr);
        pzs[idx] = fin->Get<TH1D>(zkey);
        pzs[idx]->SetDirectory(nullptr);
    }

    ProjMap nxs, nzs;
    for(const auto& idx : idxs)
    {
        double w {5};
        double s {0.2};
        // X
        auto* fx {FindBestFit(pxs[idx], w, s)};
        nxs[idx] = ScaleWithFunc(pxs[idx], fx);
        // Z
        auto* fz {FindBestFit(pzs[idx], w, s)};
        nzs[idx] = ScaleWithFunc(pzs[idx], fz);
    }


    // Fit to contour
    double thresh {0.65};
    double width {15};
    // X
    auto ypoints {FitToCountour(nxs, thresh, width)};
    // Z
    auto zpoints {FitToCountour(nzs, thresh, width)};


    // Build class to store them
    auto* smatrix {new ActPhysics::SilMatrix {"side"}};
    for(auto& [i, ypoint] : ypoints)
    {
        const auto& zpoint {zpoints[i]};
        smatrix->AddSil(i, ypoint, zpoint);
    }

    // plot
    auto* cpx {new TCanvas("cpx", "X projection canvas")};
    PlotAll(cpx, pxs);

    auto* cnx {new TCanvas("cnx", "Normalized X canvas")};
    PlotAll(cnx, nxs);

    auto* cpz {new TCanvas("cpz", "Z projection canvas")};
    PlotAll(cpz, pzs);

    auto* cnz {new TCanvas("cnz", "Normalized Z canvas")};
    PlotAll(cnz, nzs);


    auto* cm {new TCanvas("cm", "Sil matrix")};
    smatrix->Write(("./Outputs/side_matrix.root"));
    smatrix->Draw(false);
}
