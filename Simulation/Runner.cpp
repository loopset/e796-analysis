#include "TString.h"

#include <stdexcept>
#include <string>
#include <vector>

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
    std::string beam {"20O"};
    std::string target {"1H"};
    std::string light {"2H"};
    std::string heavy {"19O"};
    // Phase space reactions: when the heavy decays by proton or neutron emission
    // So we have something like: 4He + n + 17N (needs to be simulated to be included as background in fits)
    int neutronPS {0}; // number of neutrons in final state
    int protonPS {0};  // number of protons in final state
    double T1 {35};    // Beam energy: 35 MeV / u

    std::vector<double> Eexs;
    if(neutronPS == 0 && protonPS == 0)
    {
        // (d, 3He)
        if(target == "2H" && light == "3He")
            Eexs = {0};
        // (d, t)
        if(target == "2H" && light == "3H")
            Eexs = {0., 1.47, 3.24, 4.4, 5.2, 6.9};
        // (p, d)
        if(target == "1H" && light == "2H")
            Eexs = {0, 1.47, 3.24};
    }
    else if(neutronPS > 0 && protonPS == 0)
        Eexs = {0}; // only gs for n phase space
    else if(neutronPS == 0 && protonPS > 0)
        Eexs = {0};
    else
        throw std::runtime_error("No confs with neutronPS and protonPS enabled at the same time");

    if(what.Contains("simu"))
    {
        for(const auto& Eex : Eexs)
        {
            Simulation_E796(beam, target, light, heavy, neutronPS, protonPS, T1, Eex, standalone);
            if(standalone)
                break;
        }
    }
    if(what.Contains("plot"))
    {
        Plotter(Eexs, beam, target, light, heavy, T1, neutronPS, protonPS);
    }
}
