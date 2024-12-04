#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <fstream>

void gatconf()
{
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EReadSilMod};
    auto chain {datman.GetChain()};
    auto chain2 {datman.GetChain(ActRoot::ModeType::EFilter)};
    auto chain3 {datman.GetChain(ActRoot::ModeType::EMerge)};
    chain->AddFriend(chain2.get());
    chain->AddFriend(chain3.get());
    // ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};

    // Define column
    auto def {df.Define("gatconf", [](ActRoot::ModularData& d) { return d.Get("GATCONF"); }, {"ModularData"})
                  .Define("inconf", [](ActRoot::ModularData& d) { return d.Get("INCONF"); }, {"ModularData"})};
    // Count
    auto gat1 {def.Filter("gatconf == 1")};
    auto hmult {gat1.Define("Mult", "fClusters.size()").Histo1D("Mult")};

    // Book
    auto hgat {def.Histo1D("gatconf")};
    auto hin {def.Histo1D("inconf")};

    // Write!
    std::ofstream streamer {"./Debug/gat1.dat"};
    gat1.Foreach([&](ActRoot::MergerData& d) { streamer << d.fRun << " " << d.fEntry << '\n'; }, {"MergerData"});
    streamer.close();

    // Draw
    auto* c0 {new TCanvas {"c0", "GATCONF canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hgat->DrawClone();
    c0->cd(2);
    hin->DrawClone();
    c0->cd(3);
    hmult->DrawClone();
}
