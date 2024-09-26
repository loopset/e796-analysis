#include "ActKinematics.h"
#include "ActParticle.h"

#include "TString.h"

#include "PhysOMP.h"
void potentials()
{
    // Incoming channel: 20O + d
    ActPhysics::Particle beam {"20O"};
    ActPhysics::Particle d {"d"};
    double Ebeam {35 * d.GetAMU()};
    PhysOMP::Haixia in {beam.GetZ(), beam.GetA(), Ebeam};
    in.Print();

    // Compute equivalent energy
    ActPhysics::Kinematics kin {TString::Format("d(20O,19O)@%f", Ebeam).Data()};
    // kin.Print();
    double Eequiv {kin.ComputeEquivalentBeamEnergy()};
    std::cout << "Equivalent EBeam for 19O + t : " << Eequiv << '\n';
    ActPhysics::Particle o19 {"19O"};
    ActPhysics::Particle t {"t"};
    PhysOMP::Pang out {o19.GetZ(), o19.GetA(), Eequiv, true};
    out.Print();
}
