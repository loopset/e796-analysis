#include "TGraphErrors.h"
#include "AngComparator.h"

void fresco_g3()
{
    auto* gexp {new TGraphErrors{"../Outputs/xs/g3_xs.dat", "%lg %lg %lg"}};

    Angular::Comparator comp {"g3 = 3-??", gexp};
    comp.Add("iblock=2", "../Inputs/g3_Daeh/l3/fort.202");
    comp.Add("iblock=4", "./Inputs/fresco_g3/fort.204");
    comp.DrawTheo();

}
