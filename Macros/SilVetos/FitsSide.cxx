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


    // Read limits
    auto [ylimits, zlimits] {ReadFile("./side_fits.dat")};

    // Fit to normalize shape
    // Y
    auto nys {FitToScaleFunc(pxs, ylimits)};
    // Z
    auto nzs {FitToScaleFunc(pzs, zlimits)};


    // Fit to contour
    double thresh {0.65};
    double width {15};
    // Y
    auto ypoints {FitToCountour(nys, thresh, width)};
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
    auto* cpx {new TCanvas("cpy", "X projection canvas")};
    PlotAll(cpx, pxs);

    auto* cnx {new TCanvas("cny", "Normalized X canvas")};
    PlotAll(cnx, nys);

    auto* cpz {new TCanvas("cpz", "Z projection canvas")};
    PlotAll(cpz, pzs);

    auto* cnz {new TCanvas("cnz", "Normalized Z canvas")};
    PlotAll(cnz, nzs);


    auto* cm {new TCanvas("cm", "Sil matrix")};
    smatrix->Write(("./Outputs/side_matrix.root"));
    smatrix->Draw(false);
}
