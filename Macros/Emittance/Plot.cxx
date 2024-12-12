#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
void Plot()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Emittance_Tree", "./Outputs/emittance.root"};

    // Define models
    ROOT::RDF::TH2DModel mEmittance {"hEmitt", "Emittance;Y [pad];Z [btb]", 500, 0, 256, 300, 0, 256};
    auto hBegin {df.Histo2D(mEmittance, "AtBegin.fCoordinates.fY", "AtBegin.fCoordinates.fZ")};
    auto hEnd {df.Histo2D(mEmittance, "AtEnd.fCoordinates.fY", "AtEnd.fCoordinates.fZ")};
    auto hPointZ {df.Histo1D("Line.fPoint.fCoordinates.fZ")};
    auto hDirZ {df.Histo1D("Line.fDirection.fCoordinates.fZ")};

    // Plot
    auto* c0 {new TCanvas {"c0", "Emittance canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hBegin->DrawClone("colz");
    c0->cd(2);
    hEnd->DrawClone("colz");
    c0->cd(3);
    hPointZ->DrawClone();
    c0->cd(4);
    hDirZ->DrawClone();
}
