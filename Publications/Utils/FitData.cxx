#ifndef FitData_cxx
#define FitData_cxx

#include "FitData.h"

#include "Rtypes.h"

#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "THStack.h"
#include "TLegend.h"
#include "TList.h"
#include "TString.h"

#include "PhysColors.h"

#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

void PubUtils::FitData::Parse(const std::string& file, const std::string& key)
{
    auto f {std::make_unique<TFile>(file.c_str())};
    // Ex histogram
    fHist = f->Get<TH1D>("HistoEx");
    fHist->SetDirectory(nullptr);
    // Global fit
    fGlobal = f->Get<TGraph>("GraphGlobal");
    // Individual peaks
    fPeakNames = *(f->Get<std::vector<std::string>>("NamePeaks"));
    // Sort them
    SortNames(fPeakNames);
    auto list {f->Get<TList>("HistoPeaks")};
    fPeaks = new TList;
    for(const auto& peak : fPeakNames)
    {
        auto* o {list->FindObject(TString::Format("h%s", peak.c_str()))};
        ((TH1D*)o)->SetDirectory(nullptr);
        fPeaks->Add(o);
    }
    delete list;
    // Legend
    fLegend = new TLegend {0.6, 0.65, 0.95, 0.9};
    fLegend->SetFillStyle(0);
    fLegend->AddEntry(fHist, "Exp.", "lpe");
    fLegend->AddEntry(fGlobal, "Fit", "l");
}

void PubUtils::FitData::Print() const
{
    std::cout << "---- FitData ----" << '\n';
    std::cout << " fHist with " << fHist->GetEntries() << " nentries" << '\n';
    std::cout << " fPeaks with :" << '\n';
    for(const auto& name : fPeakNames)
        std::cout << "   " << name << '\n';
}

void PubUtils::FitData::BuildStack()
{
    fStack = new THStack;
    int idx {};
    for(auto* o : *fPeaks)
    {
        auto* h {(TH1D*)o};
        auto col {gPhysColors->Get(idx, "mpl")};
        h->SetLineColor(col);
        auto fs {1001};
        // auto fs {3000 + 300 + (idx < 10 ? idx * 10 + 5 : 50 + 10 - idx)};
        // std::cout << "Idx : " << idx << " fs: " << fs << '\n';
        h->SetFillStyle(fs);
        h->SetFillColorAlpha(col, 0.15);
        fStack->Add(h);
        idx++;
    }
}

void PubUtils::FitData::SetOpts(Opts opts)
{
    // Histogram title
    if(opts.count("title"))
    {
        auto str {std::any_cast<const char*>(opts["title"])};
        fHist->SetTitle(str);
    }
    fHist->SetLineWidth(2);
    fHist->SetLineColor(kBlack);
    // Ex range
    if(opts.count("rangex"))
    {
        auto pair {std::any_cast<std::pair<double, double>>(opts["rangex"])};
        fHist->GetXaxis()->SetRangeUser(pair.first, pair.second);
    }
    if(opts.count("rangey"))
    {
        auto pair {std::any_cast<std::pair<double, double>>(opts["rangey"])};
        fHist->GetYaxis()->SetRangeUser(pair.first, pair.second);
    }
    // Global fit
    if(opts.count("color"))
    {
        auto color {std::any_cast<int>(opts["color"])};
        fGlobal->SetLineColor(color);
    }
    fGlobal->SetLineWidth(3);
    // Legend
    if(opts.count("labels"))
    {
        auto map {std::any_cast<std::map<std::string, std::string>>(opts["labels"])};
        fLegend->SetNColumns((map.size() + 2) / 4 + 1);
        for(int i = 0; i < fPeakNames.size(); i++)
        {
            auto& peak {fPeakNames[i]};
            auto hist {fPeaks->At(i)};
            if(map.count(peak))
                fLegend->AddEntry(hist, map[peak].c_str(), "lf");
        }
    }
}

void PubUtils::FitData::Draw()
{
    fHist->Draw("histe");
    fGlobal->Draw("l");
    BuildStack();
    fStack->Draw("hist nostack same");
    fLegend->Draw();
}

#endif // !FitData_cxx
