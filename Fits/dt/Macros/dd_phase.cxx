#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"

#include "../../../Selector/Selector.h"

void dd_phase()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -4)};

    // Gate
    auto gated {df.Filter("ESil0 > 8.5")};

    auto hAll {df.Histo1D("Eex", "weight")};
    auto hKin {df.Histo2D({"hKin", "kin;#theta_{Lab} [#circ];E_{lab} [MeV]", 300, 0, 60, 300, 0, 20}, "theta3Lab",
                          "EVertex", "weight")};
    auto hGated {gated.Histo1D("Eex", "weight")};

    // Draw
    auto* c0 {new TCanvas {"c0", "dd_phase"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hAll->DrawNormalized();
    hGated->SetLineColor(8);
    hGated->DrawNormalized("same");
    c0->cd(2);
    hKin->DrawClone("colz");
}
