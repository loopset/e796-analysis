#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <fstream>
#include <utility>
void AntiVetoFront()
{
    // ROOT::EnableImplicitMT();
    ActRoot::JoinData join {"../configs/merger.runs"};
    ROOT::RDataFrame d {*join.Get()};

    // VETO using only ESi0 > 0, suitable for
    // 3He and 4He reactions
    // Get ANTIVETO condition
    std::set<int> maskedSiN {1, 6, 9}; //{1, 6, 9};
    auto antiveto {d.Filter(
        [&](const ROOT::VecOps::RVec<std::string>& siL, const ROOT::RVecF& siN)
        {
            // Contains energy in F0
            bool isInF0 {std::find(siL.begin(), siL.end(), "f0") != siL.end()};
            // Contains energy in F1
            bool isInF1 {std::find(siL.begin(), siL.end(), "f1") != siL.end()};
            // Impacted silicons by index
            bool isMasked {};
            // Force coincidence of index in 0 and 1
            bool shareIndex {};
            if(isInF0 && isInF1)
            {
                shareIndex = (siN[0] == siN[1]);

                for(const auto& mask : maskedSiN)
                {
                    if(siN.front() == mask)
                        isMasked = true;
                }
            }
            return isInF0 && isInF1 && shareIndex &&!isMasked;
        },
        {"fSilLayers", "fSilNs"})};

    auto df {antiveto.Filter("fRP.fCoordinates.fX > 15")};

    // Book histograms
    int ybins {250};
    std::pair<double, double> ylims {-20, 300};
    int zbins {250};
    std::pair<double, double> zlims {-20, 300};
    auto hSP {df.Histo2D({"hSP", "SP with E0 > 0 && E1 > 0;Y [mm];Z [mm]", ybins, ylims.first, ylims.second, zbins,
                          zlims.first, zlims.second},
                         "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Write cut to file
    std::ofstream streamer {"./debug_antiveto.dat"};
    ActRoot::CutsManager<int> cut;
    cut.ReadCut(0, "./Cuts/debug_antiveto.root");
    df.Foreach(
        [&](const ActRoot::MergerData& d)
        {
            if(cut.IsInside(0, d.fSP.Y(), d.fSP.Z()))
                streamer << d.fRun << " " << d.fEntry << '\n';
        },
        {"MergerData"});
    streamer.close();

    // plot
    auto* c1 {new TCanvas("c1", "Veto mode 3He and 4He")};
    hSP->DrawClone("colz");
    cut.DrawAll();
}
