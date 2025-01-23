#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "../../../Selector/Selector.h"
#include "../../Simulation_E796.cpp"

void frontDist()
{
    ROOT::EnableThreadSafety();

    // Set particles to run
    std::string target {"1H"};
    std::string light {"2H"};
    gSelector->SetTarget(target);
    gSelector->SetLight(light);
    // And set flag to save simu files
    gSelector->SetFlag("iter_front");

    // Set vector of energies
    std::vector<double> Exs {0}; // we can compare only gs and excited state at 3 MeV

    // And now distances
    std::vector<double> dists;
    double padSize {2}; // mm
    double pad {256};   // mm
    for(double d = 90; d <= 165; d += 5)
    {
        auto dist {pad + d};
        dists.push_back(dist / padSize);
    }

    // Add worker function
    auto worker {[&](double ex, double dist)
                 {
                     auto tag {TString::Format("dist_%.2f", dist)};
                     gSelector->SetTag(tag.Data());
                     gSelector->Print();
                     // Exec command
                     gSystem->Exec(TString::Format("./awk.sh %.2f", dist));
                     // And now simu!
                     Simulation_E796(gSelector->GetBeam(), gSelector->GetTarget(), gSelector->GetLight(), 0, 0, 35, ex,
                                     false);
                 }};
    // Test
    std::vector<std::thread> threads;
    for(const auto& dist : dists)
    {
        for(const auto& ex : Exs)
        {
            threads.emplace_back(worker, ex, dist);
            // Leave time for initializing all variables inside
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
    for(auto& thread : threads)
        thread.join();
}
