#include "TString.h"

#include <string>

#include "./Pipelines/Pipe1_PID.cxx"

void Runner(TString what = "1")
{
    std::string beam {"20O"};
    std::string target {"2H"};
    std::string light {"3H"};
    bool isSide {false}; // else isFront

    if(what.Contains("1"))
        Pipe1_PID(beam, target, light, isSide);
}
