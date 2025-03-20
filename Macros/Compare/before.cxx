#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include <fstream>

void before()
{
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EMerge};
    datman.SetRuns(155, 165);
    auto chain {datman.GetChain()};
    ROOT::RDataFrame df {*chain};

    // Write
    std::ofstream streamer {"before.dat"};
    df.Foreach(
        [&](ActRoot::MergerData& d)
        {
            if(d.fLightIdx != -1)
                d.Stream(streamer);
        },
        {"MergerData"});
    streamer.close();
}
