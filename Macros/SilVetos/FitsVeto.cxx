#include "ActSilMatrix.h"

#include "TFile.h"

#include <map>
#include <memory>

#include "./GetContourFuncs.cxx"

void FitsVeto()
{
    // Read data
    auto fin {std::make_unique<TFile>("./Inputs/veto_histograms.root")};
    // Set histograms to process
    std::vector<int> idxs {0, 2, 3, 4, 5, 7, 8, 10};
    std::map<int, TH1D*> pys, pzs;
    for(auto& idx : idxs)
    {
        auto ykey {TString::Format("py%d", idx)};
        auto zkey {TString::Format("pz%d", idx)};
        pys[idx] = fin->Get<TH1D>(ykey);
        pys[idx]->SetDirectory(nullptr);
        pzs[idx] = fin->Get<TH1D>(zkey);
        pzs[idx]->SetDirectory(nullptr);
    }


    // Read limits
    auto [ylimits, zlimits] {ReadFile("./antiveto_fits.dat")};

    // Fit to normalize shape
    // Y
    auto nys {FitToScaleFunc(pys, ylimits)};
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
    auto* smatrix {new ActPhysics::SilMatrix {"veto"}};
    for(auto& [i, ypoint] : ypoints)
    {
        const auto& zpoint {zpoints[i]};
        smatrix->AddSil(i, ypoint, zpoint);
    }

    // plot
    auto* cpy {new TCanvas("cpy", "Y projection canvas")};
    PlotAll(cpy, pys);

    auto* cny {new TCanvas("cny", "Normalized Y canvas")};
    PlotAll(cny, nys);

    auto* cpz {new TCanvas("cpz", "Z projection canvas")};
    PlotAll(cpz, pzs);

    auto* cnz {new TCanvas("cnz", "Normalized Z canvas")};
    PlotAll(cnz, nzs);


    auto* cm {new TCanvas("cm", "Sil matrix")};
    smatrix->Write(("./Outputs/veto_matrix.root"));
    smatrix->Draw(false);
}
