#ifndef Runner_cxx
#define Runner_cxx

#include <string>

#include "../Analyser.cxx"

void Runner()
{
    std::string beam {"16N"};
    std::string target {"2H"};
    std::string light {"3He"};
    double ebeam {640.0};
    double ex {0.0};

    auto ret {analyse(beam, target, light, ebeam, ex)};
}

#endif