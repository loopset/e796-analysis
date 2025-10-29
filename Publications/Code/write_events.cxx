#include "ActDataManager.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "TString.h"

#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
void write_events()
{
    ActRoot::DataManager datman {"../../configs/data.conf"};
    // Which events?
    // std::vector<std::pair<int, int>> events {{155, 38}, {155, 1296}, {157, 46581}, {160, 49121}};
    // std::vector<std::pair<int, int>> events {{155, 630}};
    // For thesis. One single event to highlight all Continuity and MultiAction operations
    std::vector<std::pair<int, int>> events {{155, 37333}, {156, 11522}, {156, 12164},
                                             {156, 32647}, {157, 3524},  {157, 5682}, {155, 13613}, {240, 23875}};

    for(const auto& [run, entry] : events)
    {
        datman.SetRuns(run, run);
        auto chain {datman.GetChain(ActRoot::ModeType::EReadTPC)};
        ActRoot::TPCData* data {};
        chain->SetBranchAddress("TPCData", &data);
        chain->GetEntry(entry);
        // data->Print();
        std::map<std::tuple<int, int, int>, std::pair<double, int>> map;
        // Noise
        for(const auto& v : data->fRaw)
        {
            auto& pos {v.GetPosition()};
            map[{pos.X(), pos.Y(), pos.Z()}].first += v.GetCharge();
            map[{pos.X(), pos.Y(), pos.Z()}].second = -11;
        }
        // Clusters
        for(const auto& cl : data->fClusters)
        {
            for(const auto& v : cl.GetVoxels())
            {
                auto& pos {v.GetPosition()};
                map[{pos.X(), pos.Y(), pos.Z()}].first += v.GetCharge();
                map[{pos.X(), pos.Y(), pos.Z()}].second = cl.GetClusterID();
            }
        }
        // // Write projection
        auto name {TString::Format("./Events/run_%d_entry_%d", run, entry)};
        // std::ofstream streamer {(name + ".dat").Data()};
        // for(const auto& [key, val] : map)
        // {
        //     auto [x, y, z] {key};
        //     streamer << x << " " << y << " " << z << " " << val.first << " " << val.second << '\n';
        // }
        // streamer.close();
        auto f {std::make_unique<TFile>(name + ".root", "recreate")};
        f->WriteObject(data, "TPCData");
    }
}
