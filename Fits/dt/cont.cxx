#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"

void cont()
{
    // Read the spectrum
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")};

    // Read the multiple backgrounds from other reactions
    std::vector<std::string> labels {"(d,3He) gs", "(d,3He) 5.22 MeV", "(d,a) gs", "(p,d) gs"};
    std::vector<ROOT::RDF::RNode> dfs {
        ROOT::RDataFrame {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3He", 0, -3, 0)},
        ROOT::RDataFrame {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3He", 5.22, -3, 0)},
        ROOT::RDataFrame {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "4He", 0, -3, 0)},
        ROOT::RDataFrame {"SimulationTTree", gSelector->GetSimuFile("20O", "1H", "2H", 0, -3, 0)},
    };

    // Book histograms
    auto hEx {df.Histo1D(HistConfig::Ex, "Ex")};
    std::vector<TH1D*> hs;
    int idx {};
    for(auto& node : dfs)
    {
        auto hEx {node.Histo1D(HistConfig::Ex, "Eex")};
        hEx->SetTitle(labels[idx].c_str());

        hs.push_back((TH1D*)hEx->Clone());
        idx++;
    }

    // Scale and set styles
    std::vector<int> colors {46, 8, 28, 41};
    auto integral {hEx->Integral()};
    for(int i = 0; i < hs.size(); i++)
    {
        auto& h {hs[i]};
        h->Scale(0.25 * integral / h->Integral());
        h->SetLineColor(colors[i]);
        h->SetLineWidth(2);
        h->SetLineStyle(2);
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "dt contamination"}};
    hEx->SetTitle("20O(d,t)");
    hEx->DrawClone();
    for(auto* h : hs)
    {
        h->Draw("hist same");
    }
    // Legend
    gPad->BuildLegend();
}
