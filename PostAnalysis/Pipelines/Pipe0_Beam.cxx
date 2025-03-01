#include "ActDataManager.h"
#include "ActModularData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <atomic>

void Pipe0_Beam()
{
    ROOT::EnableImplicitMT();
    // Read data
    ActRoot::DataManager datman {"/media/Data/E796v2/configs/data.conf", ActRoot::ModeType::EReadSilMod};
    auto chain {datman.GetJoinedData()};
    ROOT::RDataFrame df {*chain};

    // Get GATCONF values
    auto def {df.Define("GATCONF", [](ActRoot::ModularData& mod) { return static_cast<int>(mod.fLeaves["GATCONF"]); },
                        {"ModularData"})};

    // Book histograms
    auto hGATCONF {def.Histo1D("GATCONF")};

    // And cound CFA triggers
    std::atomic<unsigned long int> cfa {};
    def.Foreach(
        [&](const int& gatconf)
        {
            if(gatconf == 1)
                cfa++;
        },
        {"GATCONF"});

    // Draw
    auto* c0 {new TCanvas {"c00", "Pipe 0 canvas 0"}};
    hGATCONF->DrawClone();

    // Print report
    std::cout << "===== GATCONF report =====" << '\n';
    std::cout << "-> CFA/div = " << cfa << '\n';
    std::cout << "==========================" << '\n';
}
