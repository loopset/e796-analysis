#include "FitInterface.h"

void test()
{
    // Init interface
    Fitters::Interface inter;
    inter.AddState("g0", {400, 0, 0.3}, "0+");
    inter.AddState("g1", {100, 1.67, 0.3}, "0-");
    inter.AddState("ps0", {1.5});
    inter.AddState("v0", {5, 1.5, 4, 8});
    inter.AddState("cte0", {5}, "cteafafd");
    inter.EndAddingStates();
    inter.Print();

    inter.Write("./Outputs/test.root");

    Fitters::Interface read;
    read.Read("./Outputs/test.root");
    read.Print();
}
