#include "ActDataManager.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "TString.h"

#include <fstream>
#include <string>
#include <utility>
#include <vector>
void write_events()
{
    ActRoot::DataManager datman {"../../configs/data.conf"};
    // Which events?
    std::vector<std::pair<int, int>> events {{155, 38}, {155, 1296}, {157, 46581}, {160, 49121}};

    for(const auto& [run, entry] : events)
    {
        datman.SetRuns(run, run);
        auto chain {datman.GetChain(ActRoot::ModeType::EReadTPC)};
        ActRoot::TPCData* data {};
        chain->SetBranchAddress("TPCData", &data);
        chain->GetEntry(entry);
        // data->Print();
        std::map<std::tuple<int, int, int>, double> map;
        // Noise
        for(const auto& v : data->fRaw)
        {
            auto& pos {v.GetPosition()};
            map[{pos.X(), pos.Y(), pos.Z()}] += v.GetCharge();
        }
        // Clusters
        for(const auto& cl : data->fClusters)
        {
            for(const auto& v : cl.GetVoxels())
            {
                auto& pos {v.GetPosition()};
                map[{pos.X(), pos.Y(), pos.Z()}] += v.GetCharge();
            }
        }
        // Write projection
        std::ofstream streamer {TString::Format("./Events/run_%d_entry_%d.dat", run, entry).Data()};
        for(const auto& [key, val] : map)
        {
            auto [x, y, z] {key};
            streamer << x << " " << y << " " << z << " " << val << '\n';
        }
        streamer.close();
    }
}
