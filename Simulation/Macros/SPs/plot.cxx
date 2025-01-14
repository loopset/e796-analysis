#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "../../../PostAnalysis/Utils.cxx"
#include "../../../Selector/Selector.h"

void plot()
{
    double Ex {};

    ROOT::EnableImplicitMT();
    std::vector<std::string> labels {"20O(p,p)", "20O(d,d)", "20O(d,d) break"};
    std::vector<std::vector<std::string>> reaction {gSelector->GetSimuFiles("20O", "1H", "1H"),
                                                    gSelector->GetSimuFiles("20O", "2H", "2H"),
                                                    gSelector->GetSimuFiles("20O", "2H", "2H", -1)};

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

    // Get silicon matrix
    auto* sm {E796Utils::GetSideMatrix()};

    // draw
    auto* c0 {new TCanvas {"c0", "Simulation SPs"}};
    c0->DivideSquare(hsIdx.size() * 2);
    for(int i = 0; i < hsIdx.size(); i++)
    {
        c0->cd(i + 1);
        gPad->SetLogy();
        hsIdx[i]->Draw("histe");
    }
    for(int i = 0; i < hsSP.size(); i++)
    {
        c0->cd(hsIdx.size() + 1 + i);
        hsSP[i]->Draw();
        sm->DrawClone();
    }

    // Superimpose pp and d breakup
    auto* hIdxSum {(TH1D*)hsIdx.front()->Clone()};
    hIdxSum->Add(hsIdx.back(), 1);
    auto* hSPSum {(TH2D*)hsSP.front()->Clone()};
    hSPSum->Add(hsSP.back(), 1);
    auto* c1 {new TCanvas {"c1", "Superimposed histos"}};
    c1->DivideSquare(4);
    c1->cd(1);
    hIdxSum->Draw();
    c1->cd(2);
    hSPSum->DrawClone("colz");
}
