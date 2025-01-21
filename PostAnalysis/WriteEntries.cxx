#ifndef WriteEntries_cxx
#define WriteEntries_cxx

#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActSilData.h"
#include "ActSilSpecs.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include <fstream>
#include <memory>
#include <string>

#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"

void WriteEntries(const std::string& beam, const std::string& target, const std::string& light, bool isSide)
{
    ActRoot::DataManager datman {"../configs/data.conf", ActRoot::ModeType::EReadSilMod};
    datman.SetRuns(155, 156);
    auto chain {datman.GetChain()};
    auto chain2 {datman.GetChain(ActRoot::ModeType::ECorrect)};
    chain->AddFriend(chain2.get());
    ROOT::RDataFrame df {*chain};

    auto specs {std::make_shared<ActPhysics::SilSpecs>()};
    specs->ReadFile("../configs/detailedSilicons.conf");

    // Write to file
    std::ofstream streamer {"./Entries/front_up.dat"};
    df.Foreach(
        [&](ActRoot::SilData& sil, ActRoot::MergerData& data, ActRoot::ModularData& mod)
        {
            if(mod.Get("GATCONF") == 4)
            {
                sil.ApplyFinerThresholds(specs);
                if(sil.fSiN.count("f0") && sil.fSiN.size() == 1)
                    if(sil.fSiN["f0"].size() == 1)
                    {
                        // Select indexes
                        auto n {sil.fSiN["f0"].front()};
                        if(n == 8 || n == 10)
                            data.Stream(streamer);
                    }
            }
        },
        {"SilData", "MergerData", "ModularData"});
    streamer.close();
    std::cout << "Written " << df.Count().GetValue() << " entries to file!" << '\n';
}
#endif
