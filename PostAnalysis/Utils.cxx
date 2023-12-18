#ifndef Utils_cxx
#define Utils_cxx

#include "TString.h"

#include <ios>
#include <iostream>
#include <string>
namespace E796Utils
{
    TString
    GetFileName(int pipe, const std::string& beam, const std::string& target, const std::string& light, bool isSide)
    {
        auto path {TString::Format("/media/Data/E796v2/PostAnalysis/RootFiles/Pipe%d/", pipe)};
        TString name {};
        if(isSide)
            name = TString::Format("tree_beam_%s_target_%s_light_%s_side.root", beam.c_str(), target.c_str(),
                                   light.c_str());
        else
            name = TString::Format("tree_beam_%s_target_%s_light_%s_front.root", beam.c_str(), target.c_str(),
                                   light.c_str());
        return path + name;
    }

} // namespace E796Utils
#endif // !Utils_cxx
