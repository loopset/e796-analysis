#include "Interpolators.h"
#include "PhysSM.h"
void test()
{
    Interpolators::Efficiency eff;
    eff.Add("g0", "../../Simulation/Outputs/juan_RPx/tree_20O_2H_3H_0.00_nPS_0_pPS_0.root", "eff");

    Interpolators::Efficiency sca;
    sca.Add("g0", "../../Simulation/Outputs/juan_RPx/tree_20O_2H_3H_0.00_nPS_0_pPS_0.root", "eff");
    sca.Scale(0.95);

    eff.GetGraph("g0")->Draw("apl");
    sca.GetGraph("g0")->Draw("pl");

}
