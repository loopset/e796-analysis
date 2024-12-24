#include "ActSilMatrix.h"

#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TPaveText.h"
#include "TString.h"

#include <iostream>
#include <vector>

void DistDebug()
{
    auto f {new TFile {"./Outputs/Dists/sms.root"}};
    auto dists {*f->Get<std::vector<double>>("dists")};
    std::vector<ActPhysics::SilMatrix*> sms;
    for(int i = 0; i < dists.size(); i++)
    {
        sms.push_back(f->Get<ActPhysics::SilMatrix>(TString::Format("sm%d", i)));
        if(!sms.back())
            std::cout << "Nullptr" << '\n';
    }

    // Get heights per distance
    auto* gm {new TGraphErrors};
    gm->SetTitle("Mean height [1,4,7];Dist l0 [mm];Height [mm]");
    auto* gs {new TGraphErrors};
    gs->SetTitle("Mean deviation [1,4,7];Dist l0 [mm];Deviation [mm]");
    int idx {};
    for(auto& sm : sms)
    {
        std::cout << "dist : " << dists[idx] << '\n';
        // Compute std dev
        std::vector<double> heights, devs;
        for(const auto& sil : {1, 4, 7})
        {
            auto height {sm->GetHeight(sil)};
            heights.push_back(height);
            devs.push_back(TMath::Power(height - 50, 2)); // wrt nominal 50 mm height
        }
        gm->AddPoint(dists[idx], TMath::Mean(heights.begin(), heights.end()));
        auto dev {TMath::Mean(devs.begin(), devs.end())};
        dev = TMath::Sqrt(dev);
        gs->AddPoint(dists[idx], dev);
        idx++;
    }

    // Style options
    for(auto* g : {gm, gs})
    {
        g->SetMarkerStyle(24);
    }

    // Minimization of TGraphErrors
    auto xmin {TMath::MinElement(gs->GetN(), gs->GetX())};
    auto xmax {TMath::MaxElement(gs->GetN(), gs->GetX())};
    auto* func {new TF1 {"func", [=](double* x, double* p) { return gs->Eval(x[0], nullptr, "S"); }, xmin, xmax, 0}};
    auto min {func->GetMinimumX()};
    auto* text {new TPaveText {0.35, 0.7, 0.65, 0.85, "NDC"}};
    text->SetBorderSize(0);
    text->AddText(TString::Format("l_{0} = %.2f mm", min));

    // Draw
    auto* c1 {new TCanvas {"c1", "SM canvas"}};
    c1->DivideSquare(sms.size());
    for(int i = 0; i < sms.size(); i++)
    {
        c1->cd(i + 1);
        sms[i]->Draw(false);
    }
    auto* c0 {new TCanvas {"c0", "SM comparison"}};
    c0->DivideSquare(2);
    c0->cd(1);
    gm->Draw("ap");
    c0->cd(2);
    gs->Draw("ap");
    func->Draw("same");
    text->Draw();
}
