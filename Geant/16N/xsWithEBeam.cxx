#include "AngComparator.h"

void xsWithEBeam()
{
    Angular::Comparator comp;
    comp.Add("40 AMeV", "./Inputs/d3He/fort.202");
    comp.Add("20 AMeV", "./Inputs/20AMeV/fort.202");
    comp.DrawTheo();
}