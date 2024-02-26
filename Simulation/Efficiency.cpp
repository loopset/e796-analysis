#include "Interpolators.h"

#include <string>

void Efficiency()
{
    Interpolators::Efficiency effs;
    effs.Add("gs RP.X < 128", "./Outputs/Eff_study/d_t_gs_1.root");
    effs.Add("gs RP.X > 128", "./Outputs/Eff_study/d_t_gs_2.root");
    effs.Add("gs all", "./Outputs/Eff_study/d_t_gs_all.root");

    effs.Draw(true);
}
