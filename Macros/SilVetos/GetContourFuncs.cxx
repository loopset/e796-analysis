#include "ActInputParser.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TFitResult.h"
#include "TH1.h"
#include "TString.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
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
        proj->GetFunction(f->GetName())->ResetBit(TF1::kNotDraw);
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
        if(!func)
        {
            std::cout << "No scaling function for histogram " << i << '\n';
            ret[i] = h;
        }
        else
            ret[i] = ScaleWithFunc(h, func);
    }
    return std::move(ret);
}

std::pair<double, double> FitToCountour(TH1D* h, double thresh, double width)
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
    return {low, up};
}

PairMap FitToCountour(ProjMap& hs, double thresh, double width)
{
    PairMap ret;
    for(auto& [i, h] : hs)
    {
        auto [low, up] {FitToCountour(h, thresh, width)};
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

TF1* FindBestFit(TH1D* h, double width, double step, TString func = "expo")
{
    // Find guesses
    auto max {h->GetMaximum()};
    auto thresh {0.1 * max};
    auto bmin {h->FindFirstBinAbove(thresh)};
    auto bmax {h->FindLastBinAbove(thresh)};
    double minguess {h->GetBinCenter(bmin)};
    double maxguess {h->GetBinCenter(bmax)};
    // Fit for all the ranges
    std::vector<double> chis;
    std::vector<std::pair<double, double>> ranges;
    for(double a = minguess - width; a <= minguess + width; a += step)
    {
        for(double b = maxguess - width; b <= maxguess + width; b += step)
        {
            auto res {h->Fit(func, "0QSM", "", a, b)};
            chis.push_back(res.Get()->Chi2());
            ranges.push_back({a, b});
        }
    }
    // Find minimum
    auto min {std::min_element(chis.begin(), chis.end())};
    auto range {ranges[std::distance(chis.begin(), min)]};
    h->Fit(func, "0MQ", "", range.first, range.second);
    h->GetFunction(func)->ResetBit(TF1::kNotDraw);
    std::cout << "Initial guess : <" << minguess << ", " << maxguess << ">" << '\n';
    std::cout << "Best range    : <" << range.first << ", " << range.second << ">" << '\n';
    std::cout << "Chi2          : " << *min << '\n';
    auto* fit {h->GetFunction(func)};
    if(fit)
        return fit;
    else
        throw std::runtime_error("FindBestFit: nullptr after fit best model");
}
