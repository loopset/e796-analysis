#include "ActSilMatrix.h"

#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2D.h"
#include "TList.h"
#include "TString.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "./GetContourFuncs.cxx"

std::pair<double, double> Do(TH1D*& p)
{
    // Normalize
    double w {5};
    double s {0.5};
    auto* func {FindBestFit(p, w, s)};
    p = ScaleWithFunc(p, func);
    // Contour
    double thresh {0.65};
    double width {15};
    auto points {FitToCountour(p, thresh, width)};
    return points;
}

void DistPlot()
{
    auto f {std::make_unique<TFile>("./Outputs/Dists/histos.root")};
    auto dists {*f->Get<std::vector<double>>("dists")};
    // auto aux {dists[0]};
    // dists.clear();
    // dists.push_back(aux);
    // Process each one
    std::vector<TH2D*> hs2d;
    std::vector<std::map<int, TH1D*>> pxs, pzs;
    std::vector<ActPhysics::SilMatrix*> sms;
    for(const auto& dist : dists)
    {
        auto path {TString::Format("d_%.1f_mm", dist)};
        auto* dir {f->Get<TDirectory>(path)};
        auto* h2d {dir->Get<TH2D>("hSP")};
        h2d->SetDirectory(nullptr);
        hs2d.push_back(h2d);
        // Projections
        pxs.push_back({});
        pzs.push_back({});
        sms.push_back(new ActPhysics::SilMatrix);
        sms.back()->SetName(h2d->GetTitle());
        for(auto& idx : {0, 1, 2, 3, 4, 5, 6, 7})
        {
            auto xkey {TString::Format("px%d", idx)};
            auto zkey {TString::Format("pz%d", idx)};
            pxs.back()[idx] = dir->Get<TH1D>(xkey);
            pxs.back()[idx]->SetDirectory(nullptr);
            pzs.back()[idx] = dir->Get<TH1D>(zkey);
            pzs.back()[idx]->SetDirectory(nullptr);
            // Fit
            auto x = Do(pxs.back()[idx]);
            auto z = Do(pzs.back()[idx]);
            sms.back()->AddSil(idx, x, z);
        }
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "DistPlot canvas"}};
    c0->DivideSquare(hs2d.size());
    for(int i = 0; i < hs2d.size(); i++)
    {
        c0->cd(i + 1);
        hs2d[i]->Draw("colz");
    }

    // Sil matrices
    auto* c1 {new TCanvas {"c1", "SMs canvas"}};
    c1->DivideSquare(sms.size());
    for(int i = 0; i < sms.size(); i++)
    {
        c1->cd(i + 1);
        sms[i]->Draw(false);
    }

    // Plot Projections
    auto* c2 {new TCanvas {"c2", "Projection canvas"}};
    c2->DivideSquare(pzs.front().size());
    int pad {1};
    for(const auto& [_, h] : pzs.front())
    {
        c2->cd(pad);
        pad++;
        h->Draw();
    }

    // Save
    auto out {std::make_unique<TFile>("./Outputs/Dists/sms.root", "recreate")};
    out->WriteObject(&dists, "dists");
    for(int i = 0; i < sms.size(); i++)
    {
        auto name {TString::Format("sm%d", i)};
        sms[i]->SetName(name.Data());
        sms[i]->Write();
    }
}
