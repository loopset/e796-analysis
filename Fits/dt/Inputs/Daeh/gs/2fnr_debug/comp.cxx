#include "AngComparator.h"
#include "TGraphErrors.h"

void comp()
{
    auto* gexp {new TGraphErrors {"../../../../Outputs/xs/g0_xs.dat", "%lg %lg %lg"}};

    Angular::Comparator comp {"19O gs", gexp};
    comp.Add("Franck", "./../Franck/gs.xs");
    comp.Add("Fresco", "../fort.204");
    comp.Add("Fresco 0range", "../fresco_debug/fort.204");
    comp.Add("2fnr front (Vso / 2 as in fresco)", "./21.front");
    comp.Add("2fnr edited (Vso paper)", "./21.edited");
    comp.Fit();
    comp.Draw("2fnr comparison");
}
