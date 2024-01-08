#include "ActColors.h"

#include "TString.h"

#include <string>

#include "./Pipelines/Pipe1_PID.cxx"
#include "./Pipelines/Pipe2_Ex.cxx"
#include "./Plotter.cxx"
#include "./WriteEntries.cxx"

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

    if(what.Contains("1"))
        Pipe1_PID(beam, target, light, isSide);
    if(what.Contains("2"))
        Pipe2_Ex(beam, target, light, isSide);
    if(what.Contains("plot"))
        Plotter();
    if(what.Contains("write"))
        WriteEntries(beam, target, light, isSide);
}
