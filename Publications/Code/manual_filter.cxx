#include "ActDataManager.h"
#include "ActDetectorManager.h"
#include "ActLine.h"
#include "ActOptions.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "TString.h"
#include "TSystem.h"

#include <fstream>
#include <memory>
#include <utility>
#include <vector>

void write(int run, int entry, ActRoot::TPCData* data)
{
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
    // Write projection
    std::ofstream streamer {
        TString::Format("/media/Data/E796v2/Publications/Code/Events/run_%d_entry_%d.dat", run, entry)};
    for(const auto& [key, val] : map)
    {
        auto [x, y, z] {key};
        streamer << x << " " << y << " " << z << " " << val.first << " " << val.second << '\n';
    }
    streamer.close();
}

void write_lines(int run, int entry, ActRoot::TPCData* data)
{
    std::ofstream streamer {
        TString::Format("/media/Data/E796v2/Publications/Code/Events/lines_run_%d_entry_%d.dat", run, entry)};
    for(const auto& cl : data->fClusters)
    {
        auto idx {cl.GetClusterID()};
        auto p {cl.GetLine().GetPoint()};
        auto v {cl.GetLine().GetDirection()};
        streamer << idx << " " << p.X() << " " << p.Y() << " " << p.Z() << " " << v.X() << " " << v.Y() << " " << v.Z()
                 << '\n';
    }
    streamer.close();
}

void manual_filter()
{
    gSystem->cd("/media/Data/E796v2/");
    char opts[] {""};
    char* a {opts};
    ActRoot::Options::GetInstance(1, &a);
    ActRoot::Options::GetInstance()->SetMode(ActRoot::ModeType::EFilter);
    ActRoot::Options::GetInstance()->Print();
    // Events
    std::vector<std::pair<int, int>> events {{157, 46581}};
    // Data
    ActRoot::DataManager dataman {"./configs/data.conf", ActRoot::ModeType::EReadTPC};
    // Detector manager
    ActRoot::DetectorManager detman {ActRoot::ModeType::EFilter};
    detman.ReadDetectorFile("./configs/detector.conf");
    detman.ReadCalibrationsFile("./configs/calibration.conf");

    TStopwatch timer {};
    for(const auto& [run, entry] : events)
    {
        dataman.SetRuns(run, run);
        auto chain {dataman.GetChain()};
        chain->GetEntry(entry);
        auto tree {std::shared_ptr<TTree>(chain->GetTree())};
        detman.InitInput(tree);
        tree->GetEntry(entry);
        detman.BuildEvent(run, entry);
        auto filterData {
            dynamic_cast<ActRoot::TPCData*>(detman.GetDetector(ActRoot::DetectorType::EActar)->GetOutputFilter())};
        // filterData->Print();
        write(run, entry, filterData);
        write_lines(run, entry, filterData);
    }
}
