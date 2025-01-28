#include "TGraphErrors.h"

#include "AngComparator.h"
void comp()
{
    auto gexp {new TGraphErrors};
    gexp->AddPoint(21, 4.5);
    gexp->AddPoint(29, 2.2);
    gexp->SetMarkerStyle(24);
    Angular::Comparator c {"test", gexp};
    auto* gbef {new TGraphErrors {"./Fits/pd/Inputs/g0_FRESCO/fort.202", "%lg %lg"}};
    gbef->SetLineColor(8);
    gbef->SetLineWidth(2);
    c.Replace("g", gbef);
    c.Fit();
    c.Draw();

    // gbef->Draw("al");
    // gafter->Draw("l");
    // gexp->Draw("p");
}
