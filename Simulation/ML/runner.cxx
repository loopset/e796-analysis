#include "TString.h"
#include "TSystem.h"

#include <string>
#include <vector>

#include "./simu.cxx"
void runner()
{
    std::string beam {"11Li"};
    std::string target {"2H"};
    // std::vector<std::string> lights {"1H", "2H", "3H"};
    std::vector<std::string> lights {"3H"};
    double Tbeam {81.5};
    double Ex {0};

    if(lights.size() > 1)
    {
        for(const auto& light : lights)
        {
            auto exec {TString::Format("root -l -b -q 'simu.cxx(\"%s\", \"%s\", \"%s\", %.4f, %.4f)'", beam.c_str(),
                                       target.c_str(), light.c_str(), Tbeam, Ex)};
            gSystem->Exec(exec);
        }
    }
    else if(lights.size() == 1)
    {
        simu(beam, target, lights.front(), Tbeam, Ex);
    }
}
