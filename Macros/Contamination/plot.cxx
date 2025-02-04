#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "THStack.h"
#include "TROOT.h"
#include "TVirtualPad.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"

void plot()
{
    // Read the dfs
    gSelector->SetFlag("all_z1");

    ROOT::EnableImplicitMT();
    std::vector<std::string> labels {"As (p,d)", "As (d,t)"};
    std::vector<ROOT::RDF::RNode> dfs {
        ROOT::RDataFrame {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "2H")},
        ROOT::RDataFrame {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")},
    };

    auto* stack {new THStack};
    stack->SetTitle("Z = 1;E_{x} [MeV];Counts / 150 keV");
    std::vector<TH1D*> hs;
    int idx {};
    for(auto& df : dfs)
    {
        auto hEx {df.Histo1D(HistConfig::Ex, "Ex")};
        hEx->SetTitle(labels[idx].c_str());

        hs.push_back((TH1D*)hEx->Clone());
        hs.back()->SetLineWidth(2);
        stack->Add(hs.back());
    }

    // draw
    auto* c0 {new TCanvas {"c0", "Contamination canvas"}};
    stack->Draw("nostack plc");
    gPad->BuildLegend();
}
