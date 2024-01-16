#include "ActColors.h"

#include "TROOT.h"
#include "TString.h"

#include <iostream>
#include <string>

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
    std::string beam {"20O"};
    std::string target {"2H"};
    std::string light {"2H"};
    bool isSide {true}; // else isFront
    // Nice print
    Print(beam, target, light, isSide, what.Data());

    auto args {TString::Format("(\"%s\", \"%s\", \"%s\", %d)", beam.c_str(), target.c_str(), light.c_str(), isSide)};
    TString path {"./Pipelines/"};
    TString func {};
    TString ext {".cxx"};
    if(what.Contains("1"))
    {
        func = "Pipe1_PID";
        gROOT->LoadMacro(path + func + ext);
        gROOT->ProcessLine(func + args);
    }
    if(what.Contains("2"))
    {
        func = "Pipe2_Ex";
        gROOT->LoadMacro(path + func + ext);
        gROOT->ProcessLine(func + args);
    }
    if(what.Contains("plot"))
    {
        func = "Plotter";
        gROOT->LoadMacro("./" + func + ext);
        gROOT->ProcessLine(func + "()");
    }
    if(what.Contains("write"))
    {
        func = "WriteEntries";
        gROOT->LoadMacro("./" + func + ext);
        gROOT->ProcessLine(func + args);
    }
}
