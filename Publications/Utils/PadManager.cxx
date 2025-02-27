#ifndef PadManager_cxx
#define PadManager_cxx

#include "PadManager.h"

#include "TCanvas.h"
#include "TPad.h"
#include "TPaveText.h"
#include "TString.h"
#include "TStyle.h"
#include "TVirtualPad.h"

#include <iostream>
#include <vector>

void PubUtils::PadManager::Init(int npads)
{
    // Clear
    for(auto& p : fPads)
        delete p;
    fPads.clear();
    if(fCanv)
    {
        delete fCanv;
        fCanv = nullptr;
    }
    // Initialize
    fCanv = new TCanvas {"canv", "Manual canvas"};
    // Rows
    auto nrows {static_cast<int>(npads / 2)};
    // Columns (always 2 unless npads == 3)
    auto ncols {(npads == 3) ? 3 : 2};
    // Pad sizes
    double width {1. / ncols - (fxleft + fxright)};
    double height {1. / nrows - (fylow + fyup)};
    std::cout << "Nrows : " << nrows << '\n';
    std::cout << "Width : " << width << '\n';
    std::cout << "Height : " << height << '\n';
    // Create them
    for(int i = 0; i < nrows; i++)
    {
        for(int j = 0; j < ncols; j++)
        {
            auto x0 {(j + 1) * fxleft + j * (fxright + width)};
            auto x1 {x0 + width};
            std::cout << "==========" << '\n';
            std::cout << "x0 : " << x0 << " x1 : " << x1 << '\n';
            auto y0 {1 - ((i + 1) * fylow + i * (fyup + height))}; // start populating rows from the top!
            auto y1 {y0 - height};                                 // y1 -> y0 now
            std::cout << "y0 : " << y0 << " y1 : " << y1 << '\n';
            auto* p {new TPad {TString::Format("p%d%d", i, j), TString::Format("Pad i%d j%d", i, j), x0, y1, x1, y0}};
            // p->SetFillColor(29);
            // Set in-pad margins
            p->SetLeftMargin(0.15);
            p->SetRightMargin(0.03);
            p->Draw();
            fPads.push_back(p);
        }
    }
}

TPad* PubUtils::PadManager::GetPad(int idx) const
{
    auto* p {fPads.at(idx)};
    p->cd();
    return p;
}

TPad* PubUtils::PadManager::GetPad(int i, int j) const
{
    auto* o {fCanv->GetListOfPrimitives()->FindObject(TString::Format("p%d%d", i, j))};
    if(o)
    {
        auto* p {(TPad*)o};
        p->cd();
        return p;
    }
    return nullptr;
}

void PubUtils::PadManager::CenterXTitle()
{
    auto left {gPad->GetLeftMargin()};
    auto right {gPad->GetRightMargin()};
    auto width {1. - left - right};
    gStyle->SetTitleX(left + width / 2);
}

void PubUtils::PadManager::SetMargins(int row, int col, double l, double r, double b, double t)
{
    // Select pads
    std::vector<TPad*> pads;
    if(row != -1 && col == -1)
    {
        for(const auto& pad : fPads)
        {
            TString name {pad->GetName()[1]};
            if(name.Atoi() == row)
                pads.push_back(pad);
        }
    }
    else if(row == -1 && col != -1)
    {
        for(const auto& pad : fPads)
        {
            TString name {pad->GetName()[2]};
            if(name.Atoi() == col)
                pads.push_back(pad);
        }
    }
    else if(row != -1 && col != -1)
    {
        for(const auto& pad : fPads)
        {
            TString r {pad->GetName()[1]};
            TString c {pad->GetName()[2]};
            if(r.Atoi() == row && c.Atoi() == col)
                pads.push_back(pad);
        }
    }
    // Set margins
    for(const auto& pad : pads)
    {
        pad->SetLeftMargin(l == -1 ? pad->GetLeftMargin() : l);
        pad->SetRightMargin(r == -1 ? pad->GetRightMargin() : r);
        pad->SetBottomMargin(b == -1 ? pad->GetBottomMargin() : b);
        pad->SetTopMargin(t == -1 ? pad->GetTopMargin() : t);
    }
}


void PubUtils::PadManager::AddXTitle(double w, double h, const std::string& text, double x, double ts)
{
    // Create pad for this
    auto* pad {new TPad {"titlex", "titlex", x - w / 2, 0.0, x + w / 2, h}};
    pad->SetFillStyle(0);
    auto* pave {new TPaveText {0, 0, 1, 1}};
    pave->AddText(text.c_str());
    pave->SetFillStyle(0);
    pave->SetBorderSize(0);
    pave->SetTextFont(gStyle->GetTitleFont("X"));
    pave->SetTextSize(ts);
    fCanv->cd();
    pad->Draw();
    pad->cd();
    pave->Draw();
    fCanv->cd();
}

void PubUtils::PadManager::AddYTitle(double w, double h, const std::string& text, double y, double ts)
{
    // Create pad for this
    auto* pad {new TPad {"titley", "titley", 0, y - w / 2, h,  y + w / 2}};
    pad->SetFillStyle(0);
    auto* pave {new TPaveText {0, 0, 1, 1}};
    pave->AddText(text.c_str());
    pave->SetFillStyle(0);
    pave->SetBorderSize(0);
    pave->SetTextFont(gStyle->GetTitleFont("Y"));
    pave->SetTextSize(ts);
    fCanv->cd();
    pad->Draw();
    pad->cd();
    pave->Draw();
    auto* line {pave->GetLineWith(text.c_str())};
    if(line)
    {
        line->SetTextAngle(90);
        pad->Modified();
    }
    fCanv->cd();
}
#endif // !PadManager_cxx
