#include "TROOT.h"

#include <iostream>
#include <utility>
#include <vector>

#include "../Selector/Selector.h"

void All()
{
    gROOT->LoadMacro("./Runner.cxx");
    gROOT->SetBatch();
    // Run all channels
    gSelector->SetBeam("20O");
    // Vector with channel configuration
    std::vector<std::pair<std::string, std::string>> channels {{"p", "p"}, {"d", "d"}, {"p", "d"}, {"d", "t"}};
    for(const auto& [t, l] : channels)
    {
        std::cout << "Run for target : " << t << " and light  : " << l << '\n';
        gSelector->SetTarget(t);
        gSelector->SetLight(l);
        // Run root
        gROOT->ProcessLine("Runner(\"123plot\")");
    }
}
