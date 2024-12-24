#ifndef WriteEntries_cxx
#define WriteEntries_cxx

#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"
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
    datman.SetRuns(155, 165);
    auto chain {datman.GetChain()};
    auto chain2 {datman.GetChain(ActRoot::ModeType::ECorrect)};
    chain->AddFriend(chain2.get());
    ROOT::RDataFrame df {*chain};

    auto specs {std::make_shared<ActPhysics::SilSpecs>()};
    specs->ReadFile("../configs/detailedSilicons.conf");

    // Write to file
    std::ofstream streamer {"./Entries/sil2_side.dat"};
    df.Foreach(
        [&](ActRoot::SilData& sil, ActRoot::MergerData& data)
        {
            sil.ApplyFinerThresholds(specs);
            if(sil.fSiN.count("l0") && sil.fSiN.size() == 1)
                if(sil.fSiN["l0"].size() == 1 && sil.fSiN["l0"].front() == 2)
                    streamer << data.fRun << " " << data.fEntry << '\n';
        },
        {"SilData", "MergerData"});
    streamer.close();
    std::cout << "Written " << df.Count().GetValue() << " entries to file!" << '\n';
}
#endif
