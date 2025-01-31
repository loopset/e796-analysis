#ifndef Pipe3_Selector_cxx
#define Pipe3_Selector_cxx

#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <string>

#include "../../Selector/Selector.h"
#include "../Gates.cxx"
#include "../HistConfig.h"

void Pipe3_Selector(const std::string& beam, const std::string target, const std::string& light, bool isEl)
{
    // Print selector config
    gSelector->Print();

    // Read data
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Final_Tree", gSelector->GetAnaFile(2, beam, target, light, false)};

    // Apply
    auto gated {df.Filter(
        [&](ActRoot::MergerData& merger)
        {
            // Apply cut in RPx
            auto rp {E796Gates::rp(merger.fRP.X())};
            bool masksilel {true};
            bool masksiltrans {true};
            if(isEl)
                masksilel = E796Gates::maskelsil(merger.fSilNs.front());
            else
                masksiltrans = E796Gates::masktranssil(merger.fSilNs.front());
            return rp && masksilel && masksiltrans;
        },
        {"MergerData"})};

    // Book histograms
    auto hRP {gated.Histo2D(HistConfig::RP, "fRP.fCoordinates.fX", "fRP.fCoordinates.fY")};
    auto hSP {
        gated.Histo2D(HistConfig::SP, (isEl) ? "fSP.fCoordinates.fX" : "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Save
    gated.Snapshot("Sel_Tree", gSelector->GetAnaFile(3, beam, target, light, true));

    // Draw
    auto* c30 {new TCanvas {"c30", "Pipe3 canvas 0"}};
    c30->DivideSquare(4);
    c30->cd(1);
    hRP->DrawClone("colz");
    c30->cd(2);
    hSP->DrawClone("colz");
}
#endif
