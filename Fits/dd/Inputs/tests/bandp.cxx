#include "AngComparator.h"

void bandp()
{
    // Compare xs with different bandp
    Angular::Comparator comp {"bandp comparison", nullptr};
    comp.Add("4+ over gs", "./bandp1/fort.202");
    comp.Add("4+ over 2+", "./bandp2/fort.202");
    comp.DrawTheo();
}
