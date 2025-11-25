#include "TGraphErrors.h"

#include "AngComparator.h"

void systematic_omps()
{
    auto* gexp {new TGraphErrors {"../Outputs/xs/g0_xs.dat", "%lg %lg %lg"}};
    gexp->SetName("xsg0");

    Angular::Comparator comp {"g.s", gexp};
    // Incoming
    comp.Add("Daeh + Pang", "../Inputs/Daeh/gs/fort.204");
    comp.Add("Haixia + Pang", "../Inputs/Haixia/gs/fort.204");
    comp.Add("DA1p + Pang", "../Inputs/DA1p/gs/fort.204");
    // Outgoing
    // comp.Add("Daeh + Pang", "../Inputs/Daeh/gs/fort.204");
    comp.Add("Daeh + HT1P", "../Inputs/HT1p/gs/fort.204");
    comp.Fit();
    comp.Draw("Systematic omps");
    comp.Write("g0", "./Outputs/systematic_omps.root");

}
