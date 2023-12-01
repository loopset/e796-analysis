#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <utility>
void VetoFront()
{
    ROOT::EnableImplicitMT();
    ActRoot::JoinData join {"../configs/merger.runs"};
    ROOT::RDataFrame d {*join.Get()};

    // VETO using only ESi0 > 0, suitable for
    // 3He and 4He reactions
    auto df {d.Filter(
        [](const ActRoot::MergerData& d)
        {
            auto condSize {d.fSilLayers.size() >= 1};
            if(condSize)
                return (d.fSilLayers.front() == "f0");
            else
                return false;
        }, {"MergerData"})};

    // Book histograms
    int ybins {200};
    std::pair<double, double> ylims {-20, 300};
    int zbins {200};
    std::pair<double, double> zlims {-20, 300};
    auto hSP {df.Histo2D(
        {"hSP", "SP with E0 > 0;Y [mm];Z [mm]", ybins, ylims.first, ylims.second, zbins, zlims.first, zlims.second},
        "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // plot
    auto* c1 {new TCanvas("c1", "Veto mode 3He and 4He")};
    hSP->DrawClone("colz");
}
