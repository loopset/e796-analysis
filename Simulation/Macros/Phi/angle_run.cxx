#include "TROOT.h"

#include <string>
#include <vector>

#include "../../../Selector/Selector.h"
#include "../../Simulation_E796.cpp"

void angle_run()
{
    ROOT::EnableThreadSafety();

    // Set particles to run
    std::string target {"1H"};
    std::string light {"1H"};
    double Ex {0};
    gSelector->SetTarget(target);
    gSelector->SetLight(light);

    std::vector<std::string> flags {"phi_low", "phi_middle", "phi_up"};
    gSelector->SetTag("angle_8");
    gSelector->SetOpt("angle", 8); // degree

    gROOT->SetBatch();
    for(const auto& flag : flags)
    {
        gSelector->SetFlag(flag);
        Simulation_E796("20O", gSelector->GetTarget(), gSelector->GetLight(), 0, 0, 35, Ex, false);
        break;
    }
}
