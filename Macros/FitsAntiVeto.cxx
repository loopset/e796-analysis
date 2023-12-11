#include "ActInputParser.h"
#include "ActSilMatrix.h"

#include "RtypesCore.h"

#include "TAttLine.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH1D.h"
#include "TString.h"

#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Declare types
typedef std::map<int, TH1D*> ProjMap;
typedef std::map<int, std::pair<double, double>> PairMap;

std::pair<PairMap, PairMap>
ReadFile(const std::string& file, const std::string& key1 = "Y", const std::string& key2 = "Z")
{
    ActRoot::InputParser parser {file};
    auto block {parser.GetBlock("SiliconContours")};
    auto v1 {block->GetMappedValuesVectorOf<double>(key1)};
    auto v2 {block->GetMappedValuesVectorOf<double>(key2)};

    PairMap m1, m2;
    for(const auto& [key, vec] : v1)
        m1[key] = {vec.at(0), vec.at(1)};
    for(const auto& [key, vec] : v2)
        m2[key] = {vec.at(0), vec.at(1)};
    return {m1, m2};
}

TH1D* ScaleWithFunc(TH1D* h, TF1* f)
{
    TString name {h->GetName()};
    name += "_norm";
    auto ret {(TH1D*)h->Clone(name)};
    ret->SetDirectory(nullptr);
    ret->Reset();
    for(int bin = 1; bin <= h->GetNbinsX(); bin++)
    {
        auto x {h->GetBinCenter(bin)};
        auto y {h->GetBinContent(bin)};
        auto ynew {y / std::abs(f->Eval(x))};
        ret->SetBinContent(bin, ynew);
    }
    return ret;
}

void SiliconCountourGetter(TH1* proj, double lamp, double lmin, double lmax, double ramp, double rmin, double rmax)
{
    // Build functions
    auto* fleft {new TF1("fleft", "0.5 * [0] * (1 + (TMath::Erf((x - [1]) / (TMath::Sqrt(2) * [2]))))", lmin, lmax)};
    auto* fright {new TF1("fright", "0.5 * [0] * (1 - (TMath::Erf((x - [1]) / (TMath::Sqrt(2) * [2]))))", rmin, rmax)};
    // Initial parameters
    fleft->SetParameters(lamp, (lmax + lmin) / 2, 2);
    fright->SetParameters(ramp, (rmax + rmin) / 2, 2);
    // Fit
    for(auto& f : {fleft, fright})
    {
        f->SetParLimits(1, 0, 300);
        f->SetParLimits(2, 0, 4);
        proj->Fit(f, "0QR+");
    }
}

ProjMap FitToScaleFunc(ProjMap& hs, const PairMap& limits, const TString& func = "expo")
{
    ProjMap ret;
    for(auto& [i, h] : hs)
    {
        if(!limits.count(i))
            throw std::runtime_error("No Y limits set for idx : " + std::to_string(i));
        auto [xmin, xmax] {limits.at(i)};
        h->Fit("expo", "0", "", xmin, xmax);
        auto* func {h->GetFunction("expo")};
        ret[i] = ScaleWithFunc(h, func);
    }
    return std::move(ret);
}

PairMap FitToCountour(ProjMap& hs, double thresh, double width)
{
    PairMap ret;
    for(auto& [i, h] : hs)
    {
        auto bmin {h->FindFirstBinAbove(thresh)};
        auto xmin {h->GetBinCenter(bmin)};
        auto bmax {h->FindLastBinAbove(thresh)};
        auto xmax {h->GetBinCenter(bmax)};
        SiliconCountourGetter(h, 1., xmin - width, xmin + width, 1., xmax - width, xmax + width);
        auto* fleft {h->GetFunction("fleft")};
        auto* fright {h->GetFunction("fright")};
        auto low {fleft->GetParameter(1)};
        auto up {fright->GetParameter(1)};
        ret[i] = {low, up};
        std::cout << "Idx : " << i << " left : " << low << ", right : " << up << '\n';
        std::cout << "  width : " << (up - low) << " mm" << '\n';
    }
    return std::move(ret);
}

void PlotAll(TCanvas* c, ProjMap& hs)
{
    c->DivideSquare(hs.size());
    int i {0};
    for(auto& [_, h] : hs)
    {
        i++;
        c->cd(i);
        h->Draw();
        for(auto* o : *h->GetListOfFunctions())
            if(o)
                o->Draw("same");
    }
}

void FitsAntiVeto()
{
    const std::string which {""};
    auto fin {std::make_unique<TFile>(("./RootFiles/antiveto_histograms" + which + ".root").c_str())};
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

    // Read limits
    auto [ylimits, zlimits] {ReadFile("./antiveto_fits.dat")};

    // Fit to normalize shape
    // Y
    auto nys {FitToScaleFunc(pys, ylimits)};
    // Z
    auto nzs {FitToScaleFunc(pzs, zlimits)};

    // Fit to contour
    double thresh {0.4};
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
    smatrix->Write(("./silmatrix" + which + ".root"));
    smatrix->SetSyle(true, kSolid, 2, 0);
    smatrix->Draw();
}
