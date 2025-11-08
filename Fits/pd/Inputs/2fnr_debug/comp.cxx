#include "AngComparator.h"
#include "TGraphErrors.h"

void comp()
{
    auto* gexp {new TGraphErrors {"../../Outputs/xs/g0_xs.dat", "%lg %lg %lg"}};

    Angular::Comparator comp {"(p,d)", gexp};
    comp.Add("Fresco zero range", "../g0_LEA/fort.202");
    comp.Add("2fnr front", "../g0_2FNR/21.g0");
    comp.Add("2fnr edited", "./21.edited");
    comp.Fit();
    comp.Draw("2fnr comparison");
}
