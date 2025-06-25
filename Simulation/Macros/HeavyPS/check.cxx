#include "../../../Selector/Selector.h"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

void check()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"SimulationTTree", gSelector->GetApproxSimuFile("20O", "2H", "3H", 0, 1)};
    df.Describe().Print();
    auto def {df.Define("ThetaHeavy", "Lor[1].theta() * TMath::RadToDeg()")};

    auto hHeavy {def.Histo1D("ThetaHeavy")};
    hHeavy->DrawClone();
}
