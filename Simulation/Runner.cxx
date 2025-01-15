#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"

#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "../Selector/Selector.h"
#include "./Plotter.cpp"
#include "./Simulation_E796.cpp"
// what
// if simu = runs simulation
// if plot = plots results
// standalone (only applies to simu setting):
// if true, runs only first item in Ex vector and plots in-simulation results
// if false, runs all Ex simulations but doesn't plot
void Runner(TString what = "plot", bool standalone = true)
{
    // Settings
    // Names of particles
    std::string beam {gSelector->GetBeam()};
    std::string target {gSelector->GetTarget()};
    std::string light {gSelector->GetLight()};
    gSelector->Print();
    // Phase space reactions: when the heavy decays by proton or neutron emission
    // So we have something like: 4He + n + 17N (needs to be simulated to be included as background in fits)
    int neutronPS {-2}; // number of neutrons in final state:
    // if -1, break deuteron; if -2, 20O(d,d) 1n PS but rec as 20O(p,d)
    int protonPS {0}; // number of protons in final state
    double T1 {35};   // Beam energy: 35 MeV / u

    std::vector<double> Eexs;
    if(neutronPS == 0 && protonPS == 0)
    {
        // p reactions
        if(target == "1H")
        {
            if(light == "1H")
                Eexs = {0, 1.67};
            if(light == "2H")
                Eexs = {0, 1.47, 3.24};
        }
        // d reactions
        if(target == "2H")
        {
            if(light == "2H")
                Eexs = {0, 1.6, 4.0, 5.5, 6.5, 7.6, 8.6, 9.6};
            if(light == "3H")
            {
                Eexs = {0., 1.47, 3.24, 4.4, 6.7, 7.9, 8.9, 11, 12.8, 14.9, 16};
            }
            if(light == "3He")
                Eexs = {0};
            if(light == "4He")
                Eexs = {0};
        }
    }
    else if(neutronPS != 0 && protonPS == 0)
        Eexs = {0}; // only gs for n phase space
    else if(neutronPS == 0 && protonPS > 0)
        Eexs = {0};
    else
        throw std::runtime_error("Simulation::Runner(): No confs with neutronPS and protonPS enabled at the same time");

    ROOT::EnableThreadSafety();
    std::vector<std::thread> threads;
    auto worker {[](TString str) { return gSystem->Exec(str); }};
    if(what.Contains("simu"))
    {
        for(const auto& Eex : Eexs)
        {
            if(standalone)
            {
                Simulation_E796(beam, target, light, neutronPS, protonPS, T1, Eex, standalone);
                break;
            }
            else
            {
                auto str {TString::Format(
                    "root -l -b -x -q \'Simulation_E796.cpp(\"%s\",\"%s\",\"%s\",%d,%d,%f,%f,%d)\'", beam.c_str(),
                    target.c_str(), light.c_str(), neutronPS, protonPS, T1, Eex, standalone)};
                threads.emplace_back(worker, str);
            }
        }
        if(!standalone)
        {
            for(auto& thread : threads)
                thread.join();
        }
    }
    if(what.Contains("plot"))
    {
        Plotter(Eexs, beam, target, light, T1, neutronPS, protonPS);
    }
}
