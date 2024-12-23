#include "ActSilMatrix.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TString.h"

#include <iostream>
#include <memory>
#include <vector>

void DistDebug()
{
    auto f {new TFile {"./Outputs/Dists/sms.root"}};
    auto dists {*f->Get<std::vector<double>>("dists")};
    std::vector<ActPhysics::SilMatrix*> sms;
    for(int i = 0; i < dists.size(); i++)
    {
        std::cout << "dist : " << dists[i] << '\n';
        sms.push_back(f->Get<ActPhysics::SilMatrix>(TString::Format("sm%d", i)));
        if(!sms.back())
            std::cout << "Nullptr" << '\n';
    }

    // Get width per distance
    auto* g {new TGraphErrors};
    g->SetTitle("Central heights;Dist l0 [mm];Heights [mm]");
    int idx {};
    for(auto& sm : sms)
    {
        std::cout << "dist : " << dists[idx] << '\n';
        for(const auto& sil : {1, 4, 7})
            g->AddPoint(dists[idx], sm->GetHeight(sil));
        idx++;
    }

    // Style options
    g->SetMarkerStyle(24);

    // Draw
    auto* c0 {new TCanvas {"c0", "SM comparison"}};
    g->Draw("ap");

    auto* c1 {new TCanvas {"c1", "SM canvas"}};
    c1->DivideSquare(sms.size());
    for(int i = 0; i < sms.size(); i++)
    {
        c1->cd(i + 1);
        sms[i]->Draw(false);
    }
}
