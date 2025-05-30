#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
void xs()
{
    auto* gs {new TGraphErrors {"./Inputs/xs/s12_p1i.dat", "%lg %lg"}};
    gs->SetTitle("s_{1/2}");
    auto* gp {new TGraphErrors {"./Inputs/xs/p12_p1i.dat", "%lg %lg"}};
    gp->SetTitle("p_{1/2}");

    auto* mg {new TMultiGraph};
    mg->SetTitle(";#theta_{CM} [#circ];d#sigma/d#Omega [mb/sr]");
    for(auto* g : {gs, gp})
    {
        g->SetLineWidth(2);
        mg->Add(g);
    }

    auto* c0 {new TCanvas {"c0", "xs canvas"}};
    mg->Draw("apl plc pmc");
}
