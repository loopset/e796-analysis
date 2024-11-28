
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../Selector/Selector.h"

typedef std::map<int, std::set<int>> Type;

Type ProcessFile(const std::string& file)
{
    std::ifstream streamer {file};
    int run {};
    int entry {};
    Type ret;
    while(streamer >> run >> entry)
        ret[run].insert(entry);
    return ret;
}

void Compare()
{

    // Read two files
    auto bef {ProcessFile(TString::Format("./Entries/entries_%s_%s_%s_befRPDist.dat", gSelector->GetBeam().c_str(),
                                          gSelector->GetTarget().c_str(), gSelector->GetLight().c_str())
                              .Data())};
    auto after {ProcessFile(TString::Format("./Entries/entries_%s_%s_%s_afterRPDist.dat", gSelector->GetBeam().c_str(),
                                            gSelector->GetTarget().c_str(), gSelector->GetLight().c_str())
                                .Data())};

    std::map<int, std::vector<int>> missing;
    for(const auto& [run, bs] : bef)
    {
        if(after.count(run))
        {
            missing[run] = {};
            std::set_difference(bs.begin(), bs.end(), after[run].begin(), after[run].end(),
                                std::back_inserter(missing[run]));
        }
    }
    // Write
    std::ofstream streamer {TString::Format("./Entries/diff_%s_%s_%s_RPDist.dat", gSelector->GetBeam().c_str(),
                                            gSelector->GetTarget().c_str(), gSelector->GetLight().c_str())
                                .Data()};
    int count {};
    for(const auto& [run, entries] : missing)
    {
        for(const auto& entry : entries)
        {
            streamer << run << " " << entry << '\n';
            count++;
        }
    }
    streamer.close();
    std::cout << "Written : " << count << " entries" << '\n';
}
