#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"

#include "TCanvas.h"
#include "TString.h"

#include <vector>

#include "../../../PostAnalysis/HistConfig.h"
#include "../../../PostAnalysis/Utils.cxx"
#include "../../../Selector/Selector.h"
void sections()
{
    // Read sil matrix
    auto* sm {E796Utils::GetAntiVetoMatrix()};
    // Read simulation file
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0)};
    auto hRPx {df.Histo1D(HistConfig::RPx, "RPx")};

    // Filter by RPx
    std::vector<ROOT::RDF::RResultPtr<TH2D>> hsSP;
    double step {20};
    for(double x = 40; x < 200; x += step)
    {
        auto node {df.Filter([x, step](double rpx) { return x <= rpx && rpx < x + step; }, {"RPx"})};
        auto hSP {node.Histo2D(HistConfig::SP, "SP.fCoordinates.fY", "SP.fCoordinates.fZ")};
        hSP->SetTitle(TString::Format("X #in [%.0f, %.0f)", x, x + step));
        hsSP.push_back(hSP);
    }

    // Get analysis plot without sil matrix cut
    ROOT::RDataFrame ana {"Gated", "/media/Data/E796v2/Macros/Iter/gated.root"};
    auto hRPxAna {ana.Histo1D(HistConfig::RPx, "fRP.fCoordinates.fX")};
    auto hSPAna {ana.Histo2D(HistConfig::SP, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Plot
    auto* c0 {new TCanvas {"c0", "SP per distance"}};
    c0->DivideSquare(hsSP.size());
    for(int i = 0; i < hsSP.size(); i++)
    {
        c0->cd(i + 1);
        hsSP[i]->DrawClone("colz");
        sm->DrawClone();
    }

    // Normalize
    for(auto h : {hRPx, hRPxAna})
        h->Scale(1. / h->Integral());

    auto* c1 {new TCanvas {"c1", "Ana to simu comparison"}};
    c1->DivideSquare(4);
    c1->cd(1);
    hRPxAna->SetLineColor(8);
    hRPxAna->DrawClone("hist");
    hRPx->DrawClone("hist same");
    c1->cd(2);
    hSPAna->DrawClone("colz");
    sm->DrawClone();
}
