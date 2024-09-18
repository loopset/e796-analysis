#include "PhysExperiment.h"

// Generates the different normalizations
void gen()
{
    // Number of incoming beams
    double Ntrigger {279932};
    double Ndiv {30000};
    // Length X of ACTAR used in the analysis
    // Now is 256 - (256 - 220) - 26 = 194 mm
    // double Ntp {4.5625e20}; // cm2
    // double Ntd {8.2125e20}; // cm2
    // However, if we use Juan's length of 160 mm...
    double Ntp {3.75e20}; // cm2
    double Ntd {6.75e20}; // cm2

    // Build
    PhysUtils::Experiment p {Ntp, Ntrigger, Ndiv};
    p.Print();
    p.Write("p_target.dat");

    PhysUtils::Experiment d {Ntd, Ntrigger, Ndiv};
    d.Print();
    d.Write("d_target.dat");
}
