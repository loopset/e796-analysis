#ifndef Pipe3_Selector_cxx
#define Pipe3_Selector_cxx

#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <string>

#include "../../Selector/Selector.h"
#include "../HistConfig.h"
#include "../Utils.cxx"

void Pipe3_Selector(const std::string& beam, const std::string target, const std::string& light, bool isEl)
{
    // Print selector config
    gSelector->Print();

    // Read data
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Final_Tree", E796Utils::GetFile(2, beam, target, light, isEl)};

    // Apply
    auto gated {df.Filter(
        [&](ActRoot::MergerData& merger)
        {
            // Apply cut in RPx
            auto rpx {merger.fRP.X()};
            if(!(gSelector->GetRPxLow() <= rpx && rpx <= gSelector->GetRPxUp()))
                return false;
            return true;
        },
        {"MergerData"})};
    // Book histograms
    auto hRP {gated.Histo2D(HistConfig::RP, "fRP.fCoordinates.fX", "fRP.fCoordinates.fY")};

    // Save
    gated.Snapshot("Sel_Tree", E796Utils::GetFile(3, beam, target, light, isEl, gSelector->GetFlag()));

    // Draw
    auto* c30 {new TCanvas {"c30", "Pipe3 canvas 0"}};
    c30->DivideSquare(4);
    c30->cd(1);
    hRP->DrawClone("colz");
}
#endif
