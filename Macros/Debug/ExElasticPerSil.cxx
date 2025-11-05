#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "THStack.h"
#include "TString.h"
#include "TVirtualPad.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"

THStack* fill_stack(TString file)
{
    ROOT::RDataFrame df {"Sel_Tree", file.Data()};

    // Create histograms per silicon
    std::map<int, std::shared_ptr<TH1D>> hs;
    std::vector<int> idxs {0, 1, 2, 3, 4, 5, 6, 7};
    for(const auto& idx : idxs)
    {
        auto h {HistConfig::Ex.GetHistogram()};
        h->SetTitle(TString::Format("Sil %d", idx));
        hs[idx] = h;
    }
    // Fill them!
    df.Foreach(
        [&](ActRoot::MergerData& mer, double ex)
        {
            if(hs.count(mer.fSilNs.front()))
                hs[mer.fSilNs.front()]->Fill(ex);
        },
        {"MergerData", "Ex"});

    // Fill a stack
    auto* stack {new THStack};
    stack->SetTitle(HistConfig::Ex.fTitle);
    for(auto& [_, h] : hs)
    {
        auto* clone {(TH1D*)h->Clone()};
        clone->SetLineWidth(2);
        stack->Add(clone);
    }
    return stack;
}

void ExElasticPerSil()
{
    TH1::AddDirectory(false);

    std::vector<TString> files {gSelector->GetAnaFile(3, "20O", "1H", "1H"),
                                gSelector->GetAnaFile(3, "20O", "2H", "2H")};
    std::vector<THStack*> stacks;
    for(const auto& file : files)
        stacks.push_back(fill_stack(file));

    // Draw
    auto* c0 {new TCanvas {"c0", "Degug elastic"}};
    c0->DivideSquare(2);
    c0->cd(1);
    gPad->SetLogy();
    stacks[0]->SetTitle("20O(p,p)");
    stacks[0]->Draw("nostack plc pmc");
    gPad->BuildLegend();
    c0->cd(2);
    gPad->SetLogy();
    stacks[1]->SetTitle("20O(d,d)");
    stacks[1]->Draw("nostack plc pmc");
    gPad->BuildLegend();
}
