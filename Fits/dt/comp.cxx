#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TString.h"

#include "/media/Data/E796v2/Selector/Selector.h"

void comp()
{
    // Read all ground state xs
    std::string peak {"g0"};

    // Init multigraph
    auto* mg {new TMultiGraph};
    mg->SetTitle("Comparison of 20O(d,t) xs;#theta_{CM} [#circ];d#sigma / d#Omega [mb / sr]");

    // Fill
    for(const auto& flag : gSelector->GetFlags())
    {
        auto* g {new TGraphErrors {TString::Format("./Outputs/g0_%s.dat", flag.c_str()), "%lg %lg %lg"}};
        g->SetLineWidth(2);
        g->SetTitle(flag.c_str());
        mg->Add(g);
    }

    // draw
    TColor::InvertPalette();
    auto* c0 {new TCanvas {"c0", "Comp xs for dt"}};
    mg->Draw("apl plc pmc");
    c0->BuildLegend();
}
