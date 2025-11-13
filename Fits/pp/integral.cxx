#include "Rtypes.h"

#include "TCanvas.h"
#include "TGraphErrors.h"

#include "AngComparator.h"
void integral()
{
    auto* gexp {new TGraphErrors {"./Outputs/xs/g0_xs.dat", "%lg %lg %lg"}};
    Angular::Comparator comp {"g.s", gexp};
    comp.Add("CH89", "./Inputs/g0_CH89/fort.201");
    comp.IntegralExp();
    comp.IntegralModel("CH89");

    // // Read theo
    // auto* theo {new TGraphErrors {"./Inputs/g0_CH89/fort.201", "%lg %lg"}};
    // theo->SetLineColor(kRed);

    // auto* c0 {new TCanvas {"c0", "integral canvas"}};
    // theo->Draw("apl");
    // comp.GetTheoGraphs().at("CH89")->Draw("pl");
}
