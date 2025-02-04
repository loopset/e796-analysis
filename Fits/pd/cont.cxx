#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TVirtualPad.h"

#include "AngIntervals.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"

void cont()
{
    // Read the spectrum
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "2H")};

    // Read the multiple backgrounds from other reactions
    std::vector<std::string> labels {"(d,3He) gs", "(d,3He) 5.22 MeV", "(d,a) gs"};
    std::vector<ROOT::RDF::RNode> dfs {
        ROOT::RDataFrame {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3He", 0, 0, -1)},
        ROOT::RDataFrame {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3He", 5.22, 0, -1)},
        ROOT::RDataFrame {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "4He", 0, 0, -1)},
    };

    Angular::Intervals ivs {1, 20, HistConfig::Ex, 2};
    dfs.back().Foreach([&](double ex, double thetaCM) { ivs.Fill(thetaCM, ex); }, {"Eex", "theta3CM"});
    ivs.Draw();

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
    std::vector<int> colors {46, 8, 28, 42};
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
    auto* c0 {new TCanvas {"c0", "pd contamination"}};
    hEx->SetTitle("20O(p,d)");
    hEx->DrawClone();
    for(auto* h : hs)
    {
        h->Draw("hist same");
    }
    // Legend
    gPad->BuildLegend();
}
