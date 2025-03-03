#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActTPCData.h"
#include "ActTypes.h"
#include "ActVoxel.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include <vector>

void buildDataset()
{
    ROOT::EnableImplicitMT();

    ActRoot::DataManager dataman {"../configs/data.conf"};
    dataman.SetRuns(155, 155);
    auto chain {dataman.GetChain()};
    auto chain2 {dataman.GetChain(ActRoot::ModeType::EReadTPC)};
    chain->AddFriend(chain2.get());
    ROOT::RDataFrame df {*chain};

    auto def {df.Define("label", [](ActRoot::MergerData& m) { return m.fLightIdx != -1; }, {"MergerData"})
                  .Define("x",
                          [](ActRoot::TPCData& d)
                          {
                              // Clusters
                              std::vector<int> ret;
                              for(const auto& cl : d.fClusters)
                                  for(const auto& v : cl.GetVoxels())
                                      ret.push_back((int)v.GetPosition().X());
                              // Noise
                              for(const auto& v : d.fRaw)
                                  ret.push_back((int)v.GetPosition().X());
                              return ret;
                          },
                          {"TPCData"})
                  .Define("y",
                          [](ActRoot::TPCData& d)
                          {
                              // Clusters
                              std::vector<int> ret;
                              for(const auto& cl : d.fClusters)
                                  for(const auto& v : cl.GetVoxels())
                                      ret.push_back((int)v.GetPosition().Y());
                              // Noise
                              for(const auto& v : d.fRaw)
                                  ret.push_back((int)v.GetPosition().Y());
                              return ret;
                          },
                          {"TPCData"})
                  .Define("z",
                          [](ActRoot::TPCData& d)
                          {
                              // Clusters
                              std::vector<int> ret;
                              for(const auto& cl : d.fClusters)
                                  for(const auto& v : cl.GetVoxels())
                                      ret.push_back((int)v.GetPosition().Z());
                              // Noise
                              for(const auto& v : d.fRaw)
                                  ret.push_back((int)v.GetPosition().Z());
                              return ret;
                          },
                          {"TPCData"})
                  .Define("q",
                          [](ActRoot::TPCData& d)
                          {
                              // Clusters
                              std::vector<int> ret;
                              for(const auto& cl : d.fClusters)
                                  for(const auto& v : cl.GetVoxels())
                                      ret.push_back((int)v.GetCharge());
                              // Noise
                              for(const auto& v : d.fRaw)
                                  ret.push_back((int)v.GetCharge());
                              return ret;
                          },
                          {"TPCData"})};

    // Save to file
    def.Snapshot("Dataset", "./dataset.root", {"x", "y", "z", "q", "label"});
}
