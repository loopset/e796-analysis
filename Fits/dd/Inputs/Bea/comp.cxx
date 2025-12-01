#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TVirtualPad.h"

void comp()
{
    // Experimental
    // Mine
    auto* gours {new TGraphErrors {"../../Outputs/xs/g0_xs.dat", "%lg %lg %lg"}};
    gours->SetTitle("E796");
    // Hers
    auto* fhers {new TFile {"./20O_dd_gs.root"}};
    fhers->ls();
    auto* ghers {fhers->Get<TGraphErrors>("xs_g0")};
    ghers->SetTitle("Bea");

    // Theoretical
    // Mine
    auto* gtours {new TGraphErrors{"../g0_Daeh/fort.201", "%lg %lg"}};
    // Hers 
    auto* gthers {new TGraphErrors {"./fort.201", "%lg %lg"}};

    auto* mg {new TMultiGraph};
    mg->SetTitle("20O(d,d);#theta_{CM} [#circ];xs [mb/sr]");
    for(auto* g : {gours, ghers})
    {
        g->SetMarkerStyle(25);
        g->SetLineWidth(2);
        mg->Add(g);
    }
    auto* mgtheo {new TMultiGraph};
    for(auto* g : {gtours, gthers})
    {
        g->SetLineWidth(2);
        mgtheo->Add(g);
    }

    auto* c0 {new TCanvas {"c0", "Bea comp"}};
    mg->Draw("ap plc pmc");
    mgtheo->Draw("l plc pmc");
    gPad->BuildLegend();
}
