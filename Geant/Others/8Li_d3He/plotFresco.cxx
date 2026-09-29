#include "ActKinematics.h"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TVirtualPad.h"

#include "AngComparator.h"

void plotFresco()
{
    Angular::Comparator comp {"theo", nullptr};
    comp.Add("0s1/2", "./HaixiaHT1p/fort.202");
    comp.Add("0p3/2", "./HaixiaHT1p/fort.203");
    comp.Add("0d5/2", "./HaixiaHT1p/fort.204");
    // comp.Add("1s1/2", "./fort.205");
    // comp.Add("0s1/2 but j = 5/2", "./fort.206");
    comp.Add("DaehPang 0s1/2", "./DaehPang/fort.202");
    // comp.Add("0s1/2", "./fort.202");

    comp.DrawTheo();

    // auto* g {new TGraphErrors {"./fort.202", "%lg %lg"}};
    // g->SetTitle("FR FRESCO;#theta_{CM} [#circ];d#sigma/d#Omega [mb/sr]");
    // g->SetLineWidth(2);
    // g->SetLineColor(2);

    // // TWOFNR
    // auto* gfnr {new TGraphErrors {"./twofnr/21.gs", "%lg %lg"}};
    // gfnr->SetTitle("ZR TWOFNR");
    // gfnr->SetLineStyle(2);
    // gfnr->SetLineWidth(2);
    // gfnr->SetLineColor(4);

    // auto* c0 {new TCanvas {"c0", "Fresco canvas"}};
    // // c0->SetLogy();
    // g->Draw("al");
    // // gfnr->Draw("l");
    // // gPad->BuildLegend();
}
