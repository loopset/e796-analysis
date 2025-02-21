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
    std::vector<std::pair<int, int>> events {{157, 46581}};

    for(const auto& [run, entry] : events)
    {
        datman.SetRuns(run, run);
        auto chain {datman.GetChain(ActRoot::ModeType::EReadTPC)};
        ActRoot::TPCData* data {};
        chain->SetBranchAddress("TPCData", &data);
        chain->GetEntry(entry);
        // data->Print();
        std::map<std::pair<int, int>, double> projection;
        // Noise
        for(const auto& v : data->fRaw)
        {
            auto& pos {v.GetPosition()};
            projection[{pos.X(), pos.Y()}] += v.GetCharge();
        }
        // Clusters
        for(const auto& cl : data->fClusters)
        {
            for(const auto& v : cl.GetVoxels())
            {
                auto& pos {v.GetPosition()};
                projection[{pos.X(), pos.Y()}] += v.GetCharge();
            }
        }
        // Write projection
        std::ofstream streamer {TString::Format("./Events/run_%d_entry_%d.dat", run, entry).Data()};
        for(int x = 0; x < 128; x++)
            for(int y = 0; y < 128; y++)
            {
                auto val {projection[{x,y}]};
                streamer << x << " " << y << " " << (val ? std::to_string(val) : "nan") << '\n';
            }
        // for(const auto& [key, val] : projection)
        //     streamer << key.first << " " << key.second << " " << val << '\n';
        streamer.close();
    }
}
