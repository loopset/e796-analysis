#include "ActColors.h"

#include "TInterpreter.h"
#include "TROOT.h"
#include "TString.h"

#include <iostream>
#include <string>

#include "../Selector/Selector.h"

bool IsAlreadyLoaded(const TString& f)
{
    // Get factory
    auto ci {gInterpreter->ClassInfo_Factory()};
    bool isFound {gInterpreter->ClassInfo_HasMethod(ci, f)};
    gInterpreter->ClassInfo_Delete(ci);
    return isFound;
}
void Print(const std::string& beam, const std::string& target, const std::string& light, bool isSide,
           const std::string& what = "")
{
    std::cout << BOLDGREEN << "···· Runner ····" << '\n';
    std::cout << "-> Beam   : " << beam << '\n';
    std::cout << "-> Target : " << target << '\n';
    std::cout << "-> Light  : " << light << '\n';
    std::cout << "-> IsSide ? " << std::boolalpha << isSide << '\n';
    std::cout << "-> What   : " << what << '\n';
    std::cout << "······························" << RESET << '\n';
}

void Runner(TString what = "plot")
{
    // std::string beam {"20O"};
    // std::string target {"2H"};
    // std::string light {"3H"};
    // bool isSide {(target == light)}; // else isFront
    // // Nice print
    // Print(beam, target, light, isSide, what.Data());
    std::string beam {gSelector->GetBeam()};
    std::string target {gSelector->GetTarget()};
    std::string light {gSelector->GetLight()};
    bool isSide {target == light};
    gSelector->Print();

    auto args {TString::Format("(\"%s\", \"%s\", \"%s\", %d)", beam.c_str(), target.c_str(), light.c_str(), isSide)};
    TString path {"./Pipelines/"};
    TString func {};
    TString ext {".cxx"};
    if(what.Contains("0"))
    {
        func = "Pipe0_Beam";
        if(!IsAlreadyLoaded(func))
            gROOT->LoadMacro(path + func + ext);
        gROOT->ProcessLine(func + "()");
    }
    if(what.Contains("1"))
    {
        func = "Pipe1_PID";
        if(!IsAlreadyLoaded(func))
            gROOT->LoadMacro(path + func + ext);
        gROOT->ProcessLine(func + args);
    }
    if(what.Contains("2"))
    {
        func = "Pipe2_Ex";
        if(!IsAlreadyLoaded(func))
            gROOT->LoadMacro(path + func + ext);
        gROOT->ProcessLine(func + args);
    }
    if(what.Contains("3"))
    {
        func = "Pipe3_Selector";
        if(!IsAlreadyLoaded(func))
            gROOT->LoadMacro(path + func + ext);
        gROOT->ProcessLine(func + args);
    }
    if(what.Contains("plot"))
    {
        func = "Plotter";
        if(!IsAlreadyLoaded(func))
            gROOT->LoadMacro("./" + func + ext);
        gROOT->ProcessLine(func + args);
    }
    if(what.Contains("write"))
    {
        func = "WriteEntries";
        if(!IsAlreadyLoaded(func))
            gROOT->LoadMacro("./" + func + ext);
        gROOT->ProcessLine(func + args);
    }
}
