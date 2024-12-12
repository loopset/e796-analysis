#include "ActTPCData.h"

#include "ROOT/RDataFrame.hxx"
void test()
{
    ROOT::RDataFrame df {"ACTAR_Filter", "/media/Data/E796v2/RootFiles/Filter/Filter_Run_0155.root"};

    df.Foreach(
        [](ActRoot::TPCData& d)
        {
            if(d.fClusters.size() == 1)
                for(auto& v : d.fClusters.front().GetVoxels())
                    if(v.GetZs().size() > 1)
                        std::cout << "Size" << '\n';
        },
        {"TPCData"});
}
