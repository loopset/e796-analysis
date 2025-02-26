#ifndef PadManager_cxx
#define PadManager_cxx

#include "PadManager.h"

#include "TCanvas.h"
#include "TPad.h"
#include "TString.h"
#include "TStyle.h"
#include "TVirtualPad.h"

#include <iostream>

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
            auto y0 {(i + 1) * fylow + i * (fyup + height)};
            auto y1 {y0 + height};
            std::cout << "y0 : " << y0 << " y1 : " << y1 << '\n';
            auto* p {new TPad {TString::Format("p%d%d", i, j), TString::Format("Pad i%d j%d", i, j), x0, y0, x1, y1}};
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

#endif // !PadManager_cxx
