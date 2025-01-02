#include "ActSilMatrix.h"


#include "TAttLine.h"
#include "TCanvas.h"

#include "../../PostAnalysis/Utils.cxx"

void CompEffPhys()
{
    // Eff sm
    auto* sm {E796Utils::GetSideMatrix()};
    auto* leg {new ActPhysics::SilMatrix};
    leg->Read("./Inputs/MatchON/side_matrix.root");
    leg->SetSyle(false, kDashed);

    auto* c1 {new TCanvas {"c1", "Comp SMs with match"}};
    leg->Draw(false);
    sm->DrawClone();
}
