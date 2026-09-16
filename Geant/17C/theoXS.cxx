#include "AngComparator.h"

void theoXS()
{
    Angular::Comparator comp;
    comp.Add("s", "./Inputs/dp/s.dat");
    comp.Add("p", "./Inputs/dp/p.dat");
    comp.Add("d", "./Inputs/dp/d.dat");
    comp.DrawTheo();
}
