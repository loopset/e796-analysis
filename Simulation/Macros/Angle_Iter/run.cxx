#include "TROOT.h"
#include "TString.h"

#include <string>
#include <vector>

#include "../../../Selector/Selector.h"
#include "../../Simulation_E796.cpp"

void run()
{
    ROOT::EnableThreadSafety();

    // Set particles to run
    std::string target {"2H"};
    std::string light {"3H"};
    double Ex {3.24};
    gSelector->SetTarget(target);
    gSelector->SetLight(light);
    // And set flag to save simu files
    gSelector->SetFlag("iter_angle");


    // Angles
    std::vector<double> angles;
    for(double a = 0; a < 10; a += 2)
        angles.push_back(a);

    // Add worker function
    gROOT->SetBatch();
    auto worker {[&](double ang)
                 {
                     auto tag {TString::Format("ang_%.2f", ang)};
                     gSelector->SetTag(tag.Data());
                     gSelector->Print();
                     // Exec command
                     gSelector->SetOpt("angle", ang); // we cannot run in multithread mode using this
                     // And now simu!
                     Simulation_E796(gSelector->GetBeam(), gSelector->GetTarget(), gSelector->GetLight(), 0, 0, 35, Ex,
                                     true);
                 }};
    // Test
    for(const auto& angle : angles)
        worker(angle);
}
