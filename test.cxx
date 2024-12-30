#include "ActDataManager.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

void test()
{
    ActRoot::DataManager data {"./configs/data.conf"};
    auto chain {data.GetChain(ActRoot::ModeType::EReadTPC)};
    ROOT::RDataFrame df {*chain};

    df.Foreach(
        [](ActRoot::TPCData& data)
        {
            for(auto it = data.fClusters.begin(); it != data.fClusters.end(); it++)
                if(it->GetUseExtVoxels())
                    std::cout << "Use default voxels...." << '\n';
        },
        {"TPCData"});
}
