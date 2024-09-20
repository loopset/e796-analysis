#ifndef Selector_cxx
#define Selector_cxx
#include "Selector.h"

#include "ActColors.h"
#include "ActInputParser.h"

#include "TString.h"

#include <algorithm>
#include <ios>
#include <iostream>
#include <mutex>

E796::Selector* E796::Selector::fInstance {nullptr};
std::mutex E796::Selector::fMutex;

E796::Selector* E796::Selector::GetInstance(const std::string& file)
{
    if(!fInstance)
    {
        std::lock_guard<std::mutex> lock {fMutex};
        fInstance = new Selector(file);
    }
    return fInstance;
}

E796::Selector::Selector(const std::string& file)
{
    ActRoot::InputParser parser {file};
    auto selec {parser.GetBlock("Selector")};
    fFlag = selec->GetString("Flag");
    // And now search the config from the other headers
    auto headers {parser.GetBlockHeaders()};
    auto it {std::find(headers.begin(), headers.end(), fFlag)};
    if(it != headers.end())
    {
        // Read!
        auto config {parser.GetBlock(*it)};
        auto vrpx {config->GetDoubleVector("RPx")};
        fRPx = {vrpx[0], vrpx[1]};
        fEnableF07 = config->GetBool("EnableF07");
    }
}

void E796::Selector::Print() const
{
    std::cout << BOLDGREEN << "::::: E796::Selector :::::" << '\n';
    std::cout << "->Flag : " << fFlag << '\n';
    std::cout << "  RPx  : [" << fRPx.first << " , " << fRPx.second << "] mm" << '\n';
    std::cout << "  F07  ? " << std::boolalpha << fEnableF07 << '\n';
}


TString
E796::Selector::GetAnaFile(int pipe, const std::string& beam, const std::string& target, const std::string& light)
{
    bool isEl {target == light};
    auto path {TString::Format("/media/Data/E796v2/PostAnalysis/RootFiles/Pipe%d/", pipe)};
    auto name {TString::Format("tree_%s_%s_%s_%s%s.root", beam.c_str(), target.c_str(), light.c_str(),
                               (isEl) ? "side" : "front", (fFlag.size()) ? ("_" + fFlag).c_str() : "")};
    std::cout << BOLDMAGENTA << "Reading ana file : " << name << RESET << '\n';
    return path + name;
}

TString E796::Selector::GetSimuFile(const std::string& beam, const std::string& target, const std::string& light,
                                    double Ex, int nPS, int pPS)
{
    TString path {"/media/Data/E796v2/Simulation/Outputs/"};
    auto name {TString::Format("tree_%s_%s_%s_%.2f_nPS_%d_pPS_%d%s.root", beam.c_str(), target.c_str(), light.c_str(),
                               Ex, nPS, pPS, (fFlag.size()) ? ("_" + fFlag).c_str() : "")};
    std::cout << BOLDMAGENTA << "Reading simu file : " << name << RESET << '\n';
    return path + name;
}
#endif
