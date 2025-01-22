#include "PhysExperiment.h"

// Generates the different normalizations
void gen()
{
    // Number of incoming beams
    double Ntrigger {279932};
    double Ndiv {30000};
    // Total ACTAR length used in the LISE calculation
    double totalLength {256}; // mm
    // Number of targets per component
    double Np {0.3125 * 1.93e21};
    double Nd {0.5625 * 1.93e21};
    // Set renormalizations
    Nd *= 0.497;
    Np *= 0.7344;
    // Exact targets depend on the actual length of ACTAR
    // considered in the analysis, accesible in the gSelector
    // Then Ntargets = actualLength / totalLength * Np or Nd
    // INFO: this is actually wrong. (22/01/2025)
    // The simulated efficiency already gates on the RP.X(),
    // so the effect of narrowing the range is already considered in it
    // No need to "overcorrect" the number of targeta

    // Build
    PhysUtils::Experiment p {1 * Np, Ntrigger, Ndiv};
    p.Print();
    p.Write("/media/Data/E796v2/Fits/norms/p_target.dat");

    PhysUtils::Experiment d {1 * Nd, Ntrigger, Ndiv};
    d.Print();
    d.Write("/media/Data/E796v2/Fits/norms/d_target.dat");
}
