#include "ActMergerData.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TVirtualPad.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../PostAnalysis/Utils.cxx"
#include "../../Selector/Selector.h"
void CountDistrib()
{
    ROOT::EnableImplicitMT();
    // Get DFs to analyze
    std::vector<ROOT::RDF::RNode> dfs {
        ROOT::RDataFrame {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "1H")},
        ROOT::RDataFrame {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "2H")},
        ROOT::RDataFrame {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "2H")},
        ROOT::RDataFrame {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")},
    };
    std::vector<std::string> labels {"20O(p,p)", "20O(d,d)", "20O(p,d)", "20O(d,t)"};

    // SM
    auto* side {E796Utils::GetSideMatrix()};
    auto* front {E796Utils::GetAntiVetoMatrix()};

    // Histograms
    auto model2d {HistConfig::SP.GetHistogram()};
    std::vector<TH1D*> hsCounts;
    std::vector<TH2D*> hsSP;
    int i {};
    for(auto& df : dfs)
    {
        bool isSide {i < 2};
        ROOT::TThreadedObject<TH1D> hCounts {"hCounts", "Counts per pad;Sil pad;Counts", 11, 0, 11};
        ROOT::TThreadedObject<TH2D> hSP {*model2d};
        df.ForeachSlot(
            [&](unsigned int slot, ActRoot::MergerData& merger)
            {
                if(merger.fSilLayers.size() == 1)
                {
                    if(merger.fSilLayers.front() == (isSide ? "l0" : "f0"))
                    {
                        auto n {merger.fSilNs.front()};
                        hCounts.Get()->Fill(n);
                        hSP.Get()->Fill(isSide ? merger.fSP.X() : merger.fSP.Y(), merger.fSP.Z());
                    }
                }
            },
            {"MergerData"});
        hsCounts.push_back((TH1D*)hCounts.Merge()->Clone());
        hsSP.push_back((TH2D*)hSP.Merge()->Clone());
        // Set titles
        hsCounts.back()->SetTitle(labels[i].c_str());
        hsSP.back()->SetTitle(labels[i].c_str());
        i++;
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "SP canvas"}};
    c0->DivideSquare(hsCounts.size() * 2);
    for(int i = 0; i < hsCounts.size(); i++)
    {
        c0->cd(i + 1);
        gPad->SetLogy();
        hsCounts[i]->Draw("histe");
    }
    for(int i = 0; i < hsSP.size(); i++)
    {
        auto isSide {i < 2};
        c0->cd(hsCounts.size() + 1 + i);
        hsSP[i]->Draw();
        if(isSide)
            side->DrawClone();
        else
            front->DrawClone();
    }
}
