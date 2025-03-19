#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include <fstream>
#include <string>

using Map = std::map<int, std::set<int>>;

Map ReadBefore(const std::string& file)
{
    std::ifstream streamer {file};
    Map ret {};
    int r {};
    int e {};
    while(streamer >> r >> e)
        ret[r].insert(e);
    return ret;
}

void after()
{
    // Read before
    auto bef {ReadBefore("./before.dat")};

    // Read now

    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EMerge};
    datman.SetRuns(155, 175);
    auto chain {datman.GetChain()};
    ROOT::RDataFrame df {*chain};

    Map map {};
    df.Foreach(
        [&](ActRoot::MergerData& d)
        {
            if(d.fLightIdx != -1)
                map[d.fRun].insert(d.fEntry);
        },
        {"MergerData"});


    // Write to file events not in now
    std::ofstream streamer {"diff.dat"};
    for(const auto& [run, set] : bef)
    {
        if(map.count(run))
        {
            auto& now {map[run]};
            for(const auto& val : set)
            {
                if(now.find(val) == now.end())
                    streamer << run << " " << val << '\n';
            }
        }
    }
    streamer.close();
}
