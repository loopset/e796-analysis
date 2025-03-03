#include "TAxis.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TLatex.h"
#include "TMultiGraph.h"
#include "TPaveText.h"
#include "TVirtualPad.h"

#include "PhysColors.h"

#include <map>
#include <string>

#include "../Utils/VPlotData.h"
void theo()
{
    // dd
    std::map<std::string, std::string> data {
        {"daeh", "./../../Fits/dd/Inputs/g0_Daehnick/fort.201"},
        {"haix", "./../../Fits/dd/Inputs/g0_Haixia/fort.201"},
        {"kd", "./../../Fits/pp/Inputs/g0_KD/fort.201"},
        {"ch89", "./../../Fits/pp/Inputs/g0_CH89/fort.201"},
    };
    PubUtils::VPlotData::Styles sts {
        {"daeh", TAttLine(gPhysColors->Get(8, "mpl"), 1, 3)},
        {"haix", TAttLine(gPhysColors->Get(2, "mpl"), 2, 3)},
        {"ch89", TAttLine(gPhysColors->Get(6, "mpl"), 1, 3)},
        {"kd", TAttLine(gPhysColors->Get(18, "mpl"), 2, 3)},
    };

    gPhysColors->Draw();
    // Multigraph
    auto* mg {new TMultiGraph};
    mg->SetTitle(";#theta_{CM} [#circ];d#sigma/d#Omega [mb/sr]");
    for(const auto& [key, file] : data)
    {
        auto* g {new TGraphErrors {file.c_str(), "%lg %lg"}};
        if(sts.count(key))
        {
            auto& s {sts[key]};
            g->SetLineColor(s.GetLineColor());
            g->SetLineWidth(s.GetLineWidth());
            g->SetLineStyle(s.GetLineStyle());
        }
        mg->Add(g, "c");
    }
    mg->SetMinimum(8);
    mg->SetMaximum(6e3);
    auto* c0 {new TCanvas {"c0", "theo canvas", 450, 350}};
    mg->Draw("ac");
    mg->GetXaxis()->SetLimits(10, 30);
    mg->GetXaxis()->SetNdivisions(507);
    gPad->Modified();
    gPad->Update();
    gPad->SetLogy();

    // Draw boxes
    auto* dd {new TPaveText {15, 12, 22.5, 190}};
    dd->SetLineColor(gPhysColors->Get(0, "mpl"));
    dd->Draw();
    auto* pp {new TPaveText {18, 350, 25, 1300}};
    pp->SetLineColor(gPhysColors->Get(16, "mpl"));
    pp->Draw();

    // text
    auto* tdd {new TLatex {23, 40, "^{20}O(d,d) exp"}};
    auto* tpp {new TLatex {18, 2050, "^{20}O(p,p) exp"}};
    for(auto* t : {tdd, tpp})
    {
        t->SetTextFont(132);
        t->SetTextSize(0.06);
        t->Draw();
    }

    for(auto* pave : {dd, pp})
    {
        pave->SetBorderSize(1);
        pave->SetLineWidth(2);
        pave->SetFillStyle(0);
        pave->SetTextFont(132);
        pave->SetTextSize(0.06);
        pave->Draw();
    }

    //Save 
    c0->SaveAs("./Outputs/theo.pdf");
}
