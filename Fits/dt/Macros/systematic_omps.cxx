#include "TGraphErrors.h"

#include "AngComparator.h"

void systematic_omps()
{
    auto* gexp {new TGraphErrors {"../Outputs/xs/g0_xs.dat", "%lg %lg %lg"}};
    gexp->SetName("xsg0");

    Angular::Comparator comp {"g.s", gexp};
    // Incoming
    comp.Add("Daeh + Pang", "../Inputs/Sys/Daeh_Pang/fort.202");
    comp.Add("Daeh + HT1p", "../Inputs/Sys/Daeh_HT1p/fort.202");
    comp.Add("DA1p + Pang", "../Inputs/Sys/DA1p_Pang/fort.202");
    comp.Add("DA1p + HT1p", "../Inputs/Sys/DA1p_HT1p/fort.202");
    comp.Add("Haixia + Pang", "../Inputs/Sys/Haixia_Pang/fort.202");
    comp.Add("Haixia + HT1p", "../Inputs/Sys/Haixia_HT1p/fort.202");
    comp.Fit();
    comp.Draw("Systematic omps", false, true, 3, nullptr, true);
    comp.Write("g0", "./Outputs/systematic_omps.root");

}
