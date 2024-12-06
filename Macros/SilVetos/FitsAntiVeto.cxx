#include "ActSilMatrix.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TString.h"

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "./GetContourFuncs.cxx"

void FitsAntiVeto()
{
    const std::string which {""};
    auto fin {std::make_unique<TFile>(("./Inputs/antiveto_histograms" + which + ".root").c_str())};
    // fin->ls();
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

    ProjMap nys, nzs;
    for(const auto& idx : idxs)
    {
        double w {5};
        double s {0.2};
        // Y
        auto* fy {FindBestFit(pys[idx], w, s)};
        nys[idx] = ScaleWithFunc(pys[idx], fy);
        // Z
        auto* fz {FindBestFit(pzs[idx], w, s)};
        nzs[idx] = ScaleWithFunc(pzs[idx], fz);
    }

    // Fit to contour
    double thresh {0.65};
    double width {15};
    // Y
    auto ypoints {FitToCountour(nys, thresh, width)};
    // Z
    auto zpoints {FitToCountour(nzs, thresh, width)};

    // Build class to store them
    auto* smatrix {new ActPhysics::SilMatrix {"antiveto"}};
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

    // Check SilMatrix works
    auto* cm {new TCanvas("cm", "Sil matrix")};
    smatrix->Write(("./Outputs/antiveto_matrix" + which + ".root"));
    smatrix->Draw(false);
}
