#include "ActKinematics.h"
#include "ActParticle.h"

#include "TString.h"

#include <iomanip>
#include <iostream>
#include <vector>
void fresco_pars()
{
    std::vector<double> exs {0, 0.871, 3.055, 3.841, 4.554, 5.083, 5.377, 5.935, 8.213, 8.703, 9.16};
    auto sn {ActPhysics::Particle("18O").GetSn()};

    std::cout << std::fixed << std::setprecision(4);
    for(const auto& ex : exs)
    {
        std::cout << "Ex : " << ex << '\n';
        auto equiv {
            ActPhysics::Kinematics(TString::Format("d(18O,17O)t@52|%.3f", ex).Data()).ComputeEquivalentBeamEnergy()};
        std::cout << "  Ebeam 17O : " << equiv << '\n';
        std::cout << "  Sn eff    : " << sn + ex << '\n';
    }
}
