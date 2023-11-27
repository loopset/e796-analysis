#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
void GetVetosFront()
{
    ROOT::EnableImplicitMT();

    // Get data from merger
    ActRoot::JoinData data {"../configs/merger.runs"};
    ROOT::RDataFrame d {*data.Get()};

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
            if(isInF0 && isInF1)
            {
                for(const auto& mask : maskedSiN)
                {
                    if(siN.front() == mask)
                        isMasked = true;
                }
            }
            return isInF0 && isInF1 && !isMasked;
        },
        {"fSilLayers", "fSilNs"})};

    // Book histograms
    int nbinsY {200};
    double minY {-10};
    double maxY {270};
    //
    int nbinsZ {200};
    double minZ {-10};
    double maxZ {300};

    auto hAnti {antiveto.Define("corrZ", "fSP.fCoordinates.fZ - 74.4")
                    .Histo2D({"hAnti", "Antiveto histogram", nbinsY, minY, maxY, nbinsZ, minZ, maxZ},
                             "fSP.fCoordinates.fY", "corrZ")};
    // Count events
    std::cout << "N events antiveto : " << antiveto.Count().GetValue() << '\n';

    // Plot old cuts
    ActRoot::CutsManager<int> cuts;
    for(int i = 0; i <= 10; i++)
        cuts.ReadCut(i, TString::Format("../Cuts/veto_sil%d.root", i));

    // plotting
    auto* c1 {new TCanvas("c1", "Veto canvas")};
    c1->DivideSquare(2);
    c1->cd(1);
    hAnti->DrawClone("colz");
    cuts.DrawAll();
}
