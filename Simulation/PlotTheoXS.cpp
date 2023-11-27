#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"

void PlotTheoXS()
{
    // Read theoretical cross sections
    auto* g1 {new TGraphErrors("./Inputs/TheoXS/5.5MeV/angp12nospin.dat", "%lg %lg")};
    g1->SetTitle("p_{1/2}");
    auto* g2 {new TGraphErrors("./Inputs/TheoXS/5.5MeV/angp32nospin.dat", "%lg %lg")};
    g2->SetTitle("p_{3/2}");
    auto* g3 {new TGraphErrors("./Inputs/TheoXS/5.5MeV/angs12nospin.dat", "%lg %lg")};
    g3->SetTitle("s_{1/2}");

    // Set line widths
    for(auto& g : {g1, g2, g3})
        g->SetLineWidth(2);

    // Multigraph
    auto* mg {new TMultiGraph()};
    mg->SetTitle("Theoretical xs @ 5.5 MeV;#theta_{CM} [#circ];d#sigma / d#Omega [mb / sr]");
    mg->Add(g1);
    mg->Add(g2);
    mg->Add(g3);

    // plotting
    auto* c1 {new TCanvas("c1", "Theoretical xs")};
    mg->Draw("apl plc pmc");
    c1->BuildLegend();
}
