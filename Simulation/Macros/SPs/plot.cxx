#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TString.h"
#include "TVirtualPad.h"

#include <initializer_list>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../../../Selector/Selector.h"

void plot()
{
    double Ex {};

    ROOT::EnableImplicitMT();
    std::vector<std::string> labels {"20O(p,p)", "20O(d,d)", "20O(d,d) break", "20O(d,t)", "20O(p,d)"};
    std::vector<std::vector<std::string>> reaction {
        gSelector->GetSimuFiles("20O", "1H", "1H"),     gSelector->GetSimuFiles("20O", "2H", "2H"),
        gSelector->GetSimuFiles("20O", "2H", "2H", -1), gSelector->GetSimuFiles("20O", "2H", "3H"),
        gSelector->GetSimuFiles("20O", "1H", "2H"),
    };

    // Run!
    std::vector<TH2D*> hsSP;
    std::vector<TH1D*> hsIdx;
    int idx {};
    for(auto files : reaction)
    {
        ROOT::RDataFrame df {"SimulationTTree", files};
        auto hIdx {df.Histo1D({"hIdx", ";Sil pad;Counts", 11, 0, 11}, "SilIdx")};
        hIdx->SetTitle(labels[idx].c_str());

        // file objects
        TH2D* hSP {};
        for(auto file : files)
        {
            auto f {std::make_unique<TFile>(file.c_str())};
            auto* haux {f->Get<TH2D>("hSP")};
            haux->SetTitle(labels[idx].c_str());
            if(hSP)
                hSP->Add(haux);
            else
            {
                hSP = (TH2D*)haux->Clone();
                hSP->SetDirectory(nullptr);
            }
            f->Close();
        }

        // add to vectors
        hsIdx.push_back((TH1D*)hIdx->Clone());
        hsSP.push_back(hSP);

        idx++;
    }

    // Drawing structure
    int np {6};
    int nc {static_cast<int>(labels.size() / np)};
    int ip {0};
    int ic {0};
    TCanvas* c {};
    for(int i = 0; i < labels.size(); i++)
    {
        std::cout << "i : " << i << " ip : " << ip << '\n';
        if(ip == 0)
        {
            c = new TCanvas {TString::Format("c%d", ic), "Canvas for count distrib"};
            c->DivideSquare(np);
        }
        c->cd(2 * ip + 1);
        gPad->SetLogy();
        hsIdx[i]->Draw("histe");
        c->cd(2 * ip + 2);
        hsSP[i]->Draw();
        ip++;
        if(ip * 2 >= np)
        {
            ip = 0;
            ic++;
        }
    }

    // // Superimpose pp and d breakup
    // auto* hIdxSum {(TH1D*)hsIdx.front()->Clone()};
    // hIdxSum->Add(hsIdx.back(), 1);
    // auto* hSPSum {(TH2D*)hsSP.front()->Clone()};
    // hSPSum->Add(hsSP.back(), 1);
    // auto* c1 {new TCanvas {"c1", "Superimposed histos"}};
    // c1->DivideSquare(4);
    // c1->cd(1);
    // hIdxSum->Draw();
    // c1->cd(2);
    // hSPSum->DrawClone("colz");
}
