#include "TString.h"
#include "TSystem.h"

#include <string>

#include "./Pipelines/Pipe1_PID.cxx"
#include "./Pipelines/Pipe2_Ex.cxx"
#include "./Plotter.cxx"

void Runner(TString what = "1")
{
    std::string beam {"20O"};
    std::string target {"1H"};
    std::string light {"2H"};
    bool isSide {false}; // else isFront

    if(what.Contains("1"))
        Pipe1_PID(beam, target, light, isSide);
    if(what.Contains("2"))
        Pipe2_Ex(beam, target, light, isSide);
    if(what.Contains("plot"))
        Plotter();
}
