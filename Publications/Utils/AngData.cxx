#ifndef AngData_cxx
#define AngData_cxx
#include "AngData.h"

#include "TAxis.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TLegend.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TPaveText.h"
#include "TString.h"
#include "TStyle.h"

#include "PhysSF.h"
#include "VPlotData.h"

#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// If we read the multigraph from the file it is impossible
// to change its attributes in an easy manner
// Therefore, we create a new one
void PubUtils::AngData::SetMulti(TMultiGraph* mg)
{
    // Multigraph:
    // When reading it from tree, it is agnostic to the gStyle
    // We have to retrieve the underlying histo and force style for it
    fMulti = (TMultiGraph*)mg->Clone();
    fMulti->GetHistogram()->UseCurrentStyle();
    fMulti->GetHistogram()->SetStats(false);
    for(auto* o : *fMulti->GetListOfGraphs())
    {
        auto* g {(TGraphErrors*)o};
        fGraphs.push_back(g);
    }
}

void PubUtils::AngData::Parse(const std::string& file, const std::string& key)
{
    auto f {std::make_unique<TFile>(file.c_str())};
    // Multigraph
    auto legacy {f->Get<TMultiGraph>((key + "_mg").c_str())};
    SetMulti(legacy);
    delete legacy;
    // Collection
    fSFs = f->Get<PhysUtils::SFCollection>((key + "_sfs").c_str());
}

void PubUtils::AngData::Print() const
{
    std::cout << "----- AngData ----" << '\n';
    std::cout << " Graph named : " << fMulti->GetTitle() << '\n';
    std::cout << " SFs content : " << '\n';
    for(const auto& name : fSFs->GetModels())
        std::cout << "   " << name << '\n';
}

void PubUtils::AngData::SetOpts(Opts opts)
{
    if(opts.count("title"))
    {
        auto title {std::any_cast<const char*>(opts["title"])};
        fMulti->GetHistogram()->SetTitle(title);
    }
    if(opts.count("titlex"))
    {
        auto title {std::any_cast<const char*>(opts["titlex"])};
        fMulti->GetHistogram()->GetXaxis()->SetTitle(title);
    }
    if(opts.count("titley"))
    {
        auto title {std::any_cast<const char*>(opts["titley"])};
        fMulti->GetHistogram()->GetYaxis()->SetTitle(title);
    }
    if(opts.count("labelx"))
    {
        auto withLabels {std::any_cast<bool>(opts["labelx"])};
        if(!withLabels)
            fMulti->GetHistogram()->GetXaxis()->SetLabelSize(0);
    }
    if(opts.count("rangex"))
    {
        auto pair {std::any_cast<std::pair<double, double>>(opts["rangex"])};
        fMulti->GetHistogram()->GetXaxis()->SetRangeUser(pair.first, pair.second);
    }
}


void PubUtils::AngData::DisableLabel(const std::string& axis, int idx)
{
    auto* ax {(axis == "x") ? fMulti->GetHistogram()->GetXaxis() : fMulti->GetHistogram()->GetYaxis()};
    ax->ChangeLabel(idx, 0, 0, 0, 0, 0, "");
}

void PubUtils::AngData::SetNDiv(const std::string& axis, int ndiv)
{
    auto* ax {(axis == "x") ? fMulti->GetHistogram()->GetXaxis() : fMulti->GetHistogram()->GetYaxis()};
    ax->SetNdivisions(ndiv);
}

void PubUtils::AngData::SetText(double x, double y, const std::string& text, double w, double h)
{
    auto* pave {new TPaveText {x - w / 2, y - h / 2, x + w / 2, y + h / 2, "NDC"}};
    pave->SetTextSize(gStyle->GetTextSize());
    pave->SetTextFont(gStyle->GetTextFont());
    pave->SetFillStyle(0);
    pave->SetBorderSize(0);
    // Find line break
    auto lw {text.find_first_of("\\")};
    auto a {text};
    std::string b {};
    if(lw != std::string::npos)
    {
        a = text.substr(0, lw);
        b = text.substr(lw + 1);
    }
    pave->AddText(a.c_str());
    if(b.length())
        pave->AddText(b.c_str());
    fMulti->GetListOfFunctions()->Add(pave);
}

void PubUtils::AngData::SetGraphsStyle(const VPlotData::Styles& styles)
{
    std::vector<double> alreadySet(fGraphs.size(), false);
    for(const auto& [key, st] : styles)
    {
        TString k {key};
        k.ToLower();
        k.ReplaceAll(" ", "");
        int idx {};
        for(const auto& g : fGraphs)
        {
            TString title {g->GetTitle()};
            title.ToLower();
            title.ReplaceAll(" ", "");
            // std::cout << "Key : " << k << " title: " << title << '\n';
            if(title.Contains(k) && !alreadySet[idx])
            {
                g->SetLineWidth(st.GetLineWidth());
                g->SetLineStyle(st.GetLineStyle());
                g->SetLineColor(st.GetLineColor());
                alreadySet[idx] = true;
            }
            idx++;
        }
    }
}

void PubUtils::AngData::GetLegendGraph(TLegend* leg, TAttLine st, const std::string& name)
{
    auto* g {new TGraphErrors};
    g->SetLineWidth(st.GetLineWidth());
    g->SetLineStyle(st.GetLineStyle());
    g->SetLineColor(st.GetLineColor());
    g->SetTitle(name.c_str());
    leg->AddEntry(g, name.c_str(), "l");
}

void PubUtils::AngData::SetRangeY(double ymin, double ymax)
{
    if(ymin != -11)
        fMulti->SetMinimum(ymin);
    if(ymax != -11)
        fMulti->SetMaximum(ymax);
}

void PubUtils::AngData::CenterY(double factor)
{
    // Experimental graph ch
    TGraphErrors* gexp {};
    for(const auto& g : fGraphs)
        if(TString name {g->GetName()}; name.Contains("xs"))
        {
            gexp = g;
            break;
        }
    auto min {TMath::MinElement(gexp->GetN(), gexp->GetY())};
    auto max {TMath::MaxElement(gexp->GetN(), gexp->GetY())};
    auto diff {max - min};
    // std::cout << "Min : " << min << " max : " << max << '\n';
    auto y0 {min - factor * diff};
    auto y1 {max + factor * diff};
    fMulti->SetMinimum(y0 < 0 ? 0 : y0);
    fMulti->SetMaximum(y1);
}

void PubUtils::AngData::Draw()
{
    fMulti->Draw("apl");
}

#endif // !AngData_cxx
