#include "ActSilMatrix.h"
#include "TAttLine.h"

#include "../PostAnalysis/Utils.cxx"
void test()
{
    auto sm {E796Utils::GetAntiVetoMatrix()};
    auto clone {sm->Clone()};
    clone->MoveXYTo(181, {135, 170}, 200);
    clone->SetSyle(false, kDashed);

    sm->Draw(false);
    clone->Draw();
}
