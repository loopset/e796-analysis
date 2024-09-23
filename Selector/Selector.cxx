#ifndef Selector_cxx
#define Selector_cxx
#include "Selector.h"

#include "ActColors.h"

#include "TROOT.h"
#include "TString.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>

void E796::Config::ReadConfig(std::shared_ptr<ActRoot::InputBlock> b)
{
    auto vrpx {b->GetDoubleVector("RPx")};
    fRPx = {vrpx[0], vrpx[1]};
    fLengthX = fRPx.second - fRPx.first;
}

void E796::Config::Print() const
{
    std::cout << "  RPx      : [" << fRPx.first << " , " << fRPx.second << "] mm" << '\n';
    std::cout << "  LengthX  : " << fLengthX << " mm" << '\n';
}

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
    // Read particles
    fBeam = selec->GetString("Beam");
    fTarget = selec->GetString("Target");
    fLight = selec->GetString("Light");
    ReassignNames();
    // Read flag
    auto flag {selec->GetString("Flag")};

    // And now read the configs
    auto headers {parser.GetBlockHeaders()};
    for(const auto& header : headers)
    {
        if(header != "Selector")
        {
            Config config {};
            config.ReadConfig(parser.GetBlock(header));
            fConfigs[header] = config;
        }
    }
    // And set configuration
    SetFlag(flag);
}

void E796::Selector::SetFlag(const std::string& flag)
{
    if(!fConfigs.count(flag))
        throw std::runtime_error("E796::Selector::SetFlag(): could not set flag " + flag);
    fFlag = flag;
    fCurrent = &(fConfigs.find(fFlag)->second);
}

void E796::Selector::Print() const
{
    std::cout << BOLDGREEN << "::::: E796::Selector :::::" << '\n';
    std::cout << "->Beam     : " << fBeam << '\n';
    std::cout << "->Light    : " << fTarget << '\n';
    std::cout << "->Target   : " << fLight << '\n';
    std::cout << "->Flag     : " << fFlag << '\n';
    fCurrent->Print();
    std::cout << "::::::::::::::::::::::::::::::" << RESET << '\n';
}


TString E796::Selector::GetAnaFile(int pipe, const std::string& beam, const std::string& target,
                                   const std::string& light, bool withFlag)
{
    bool isEl {target == light};
    auto path {TString::Format("/media/Data/E796v2/PostAnalysis/RootFiles/Pipe%d/", pipe)};
    auto name {TString::Format("tree_%s_%s_%s_%s%s.root", beam.c_str(), target.c_str(), light.c_str(),
                               (isEl) ? "side" : "front", (withFlag) ? ("_" + fFlag).c_str() : "")};
    std::cout << BOLDMAGENTA << "Opening ana file : " << name << RESET << '\n';
    return path + name;
}

TString E796::Selector::GetSimuFile(const std::string& beam, const std::string& target, const std::string& light,
                                    double Ex, int nPS, int pPS)
{
    TString path {"/media/Data/E796v2/Simulation/Outputs/"};
    auto name {TString::Format("tree_%s_%s_%s_%.2f_nPS_%d_pPS_%d%s.root", beam.c_str(), target.c_str(), light.c_str(),
                               Ex, nPS, pPS, (fFlag.size()) ? ("_" + fFlag).c_str() : "")};
    std::cout << BOLDMAGENTA << "Opening simu file : " << name << RESET << '\n';
    return path + name;
}

void E796::Selector::RecomputeNormalization() const
{
    gROOT->Macro("/media/Data/E796v2/Fits/norms/gen.cxx");
}

void E796::Selector::ReassignNames()
{
    // Target
    if(fTarget == "d")
        fTarget = "2H";
    else if(fTarget == "p")
        fTarget = "1H";
    else
        std::runtime_error("E796::Selector::Names(): fTargets differs from 1H or 2H");

    // Light
    if(fLight.find_first_of("0123456789") == std::string::npos)
    {
        if(fLight == "p")
            fLight = "1H";
        else if(fLight == "d")
            fLight = "2H";
        else if(fLight == "t")
            fLight = "3H";
        else if(fLight == "a")
            fLight = "4He";
        else
            throw std::runtime_error("E796::Selector::Names(): could not convert " + fLight + " to correct name");
    }
}

#endif
