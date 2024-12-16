#include "ActSilMatrix.h"
#include "ActSilSpecs.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TH2.h"
#include "TPaveText.h"
#include "TString.h"

#include <iostream>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <vector>

#include "../../PostAnalysis/Utils.cxx"

void OverlapSM()
{
    // Read histograms
    auto file {new TFile {"./Outputs/histos.root"}};
    auto* hxz {file->Get<TH2D>("hXZ")};
    auto* hyz {file->Get<TH2D>("hYZ")};
    // Mean of beam in histogram
    auto meanSide {hxz->GetMean(2)};
    auto meanFront {hyz->GetMean(2)};

    // Real silicon specs
    ActPhysics::SilSpecs specs;
    specs.ReadFile("../../configs/detailedSilicons.conf");

    // Read SM
    auto* veto {E796Utils::GetVetoMatrix()};
    auto* antiveto {E796Utils::GetAntiVetoMatrix()};
    auto* side {E796Utils::GetSideMatrix()};
    // Map things
    std::vector<TH2D*> hs {hxz, hyz, hyz};
    std::vector<ActPhysics::SilMatrix*> sms {side, antiveto, veto};
    std::vector<std::string> labels {"Side", "Antiveto", "Veto"};
    std::vector<std::string> layers {"l0", "f0", "f1"};
    std::vector<ActPhysics::SilMatrix*> phys;
    // Format phys sms
    for(int i = 0; i < labels.size(); i++)
    {
        auto sm {specs.GetLayer(layers[i]).GetSilMatrix()};
        phys.push_back(sm->Clone());
        phys.back()->SetName(labels[i]);
    }

    // Get means of desired silicons
    std::vector<double> zmeans;
    std::vector<double> diffs;
    std::vector<TPaveText*> texts;
    int idx {};
    for(auto* sm : sms)
    {
        TString label {labels[idx]};
        label.ToLower();
        std::vector<double> temp;
        std::set<int> sils;
        double ref {};
        if(label.Contains("side"))
        {
            sils = {4, 5};
            ref = meanSide;
        }
        else if(label.Contains("veto"))
        {
            sils = {3, 4};
            ref = meanFront;
        }
        for(auto sil : sils)
        {
            double x {};
            double y {};
            sm->GetSil(sil)->Center(x, y);
            temp.push_back(y);
        }
        // Compute mean
        zmeans.push_back(std::accumulate(temp.begin(), temp.end(), 0.0) / temp.size());
        // And diff
        diffs.push_back(zmeans.back() - ref);
        auto* text {new TPaveText {0.4, 0.75, 0.6, 0.88, "NDC"}};
        text->AddText(TString::Format("#DeltaZ = %.2f mm", diffs.back()));
        text->SetBorderSize(0);
        texts.push_back(text);
        // Print:
        std::cout << "Mean for " << labels[idx] << " : " << zmeans.back() << " mm" << '\n';
        // Move center of physical sms to this value
        phys[idx]->MoveZTo(zmeans.back(), sils);
        idx++;
    }


    // Draw
    auto* c0 {new TCanvas {"c0", "SM and Emittance canvas"}};
    c0->DivideSquare(4);
    for(int i = 0; i < labels.size(); i++)
    {
        c0->cd(i + 1);
        hs[i]->DrawCopy()->SetTitle(labels[i].c_str());
        sms[i]->Draw();
        texts[i]->Draw();
    }

    auto* c1 {new TCanvas {"c1", "Physical silicons"}};
    c1->DivideSquare(4);
    for(int i = 0; i < phys.size(); i++)
    {
        c1->cd(i + 1);
        phys[i]->Draw(false);
        auto* cl {sms[i]->Clone()};
        cl->SetSyle(false, 0, 0, 3001);
        cl->Draw();
    }
}
