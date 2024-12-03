#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActSilData.h"
#include "ActTypes.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
void SilData()
{
    ActRoot::DataManager datman {"../../configs/data.conf"};
    auto chain {datman.GetJoinedData()};
    auto chain2 {datman.GetJoinedData(ActRoot::ModeType::EReadSilMod)};
    chain->AddFriend(chain2.get());
    // ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};

    df.Foreach(
        [](const ActRoot::MergerData& d, const ActRoot::SilData& s)
        {
            // if(d.fSilLayers.size() == 2)
            //     if(d.fSilLayers.front() == "f1")
            //         std::cout << "f1 in front" << '\n';
            if(d.fSilLayers.size() == 2)
            {
                std::cout << "=========" << '\n';
                for(const auto& [name, es] : s.fSiE)
                    for(const auto& e : es)
                        std::cout << "Name : " << name << " E : " << e << '\n';
            }
        },
        {"MergerData", "SilData"});
}
