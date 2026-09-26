#include "ActKinematics.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TVirtualPad.h"

void plotFresco()
{
    auto* g {new TGraphErrors{"./fort.202", "%lg %lg"}};
    g->SetTitle("FR FRESCO;#theta_{CM} [#circ];d#sigma/d#Omega [mb/sr]");
    g->SetLineWidth(2);
    g->SetLineColor(2);

    // TWOFNR
    auto* gfnr {new TGraphErrors {"./twofnr/21.gs", "%lg %lg"}};
    gfnr->SetTitle("ZR TWOFNR");
    gfnr->SetLineStyle(2);
    gfnr->SetLineWidth(2);
    gfnr->SetLineColor(4);

    auto* c0 {new TCanvas {"c0", "Fresco canvas"}};
    // c0->SetLogy();
    g->Draw("al");
    // gfnr->Draw("l");
    // gPad->BuildLegend();
}
