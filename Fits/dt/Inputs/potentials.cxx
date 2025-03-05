#include "ActKinematics.h"
#include "ActParticle.h"

#include "TString.h"

#include "PhysOMP.h"
void potentials(double Ex = 0)
{
    // Incoming channel: 20O + d
    ActPhysics::Particle beam {"20O"};
    ActPhysics::Particle d {"d"};
    double Ebeam {35 * d.GetAMU()};
    auto Sn {beam.GetSn()};
    PhysOMP::Daehnick in {beam.GetZ(), beam.GetA(), Ebeam};
    in.Print();

    // Compute equivalent energy
    ActPhysics::Kinematics kin {TString::Format("d(20O,19O)@%f|%f", Ebeam, Ex).Data()};
    // kin.Print();
    double Eequiv {kin.ComputeEquivalentBeamEnergy()};
    std::cout << "Equivalent EBeam for 19O + t : " << Eequiv << '\n';
    std::cout << " and Ex     : " << Ex << '\n';
    std::cout << " and Sn eff = Sn + Ex : " << Sn + Ex << '\n';
    ActPhysics::Particle o19 {"19O"};
    ActPhysics::Particle t {"t"};
    PhysOMP::Pang out {o19.GetZ(), o19.GetA(), Eequiv, true};
    out.Print();
}
