#include "Interpolators.h"

#include <string>

#include "../../../Selector/Selector.h"

void elastic()
{
    std::string beam {"20O"};
    double Ex {0};

    Interpolators::Efficiency dd, pp;
    dd.Add("Juan", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/"
                   "20O_and_2H_to_2H_NumN_0_NumP_0_Ex0_Date_2024_10_2_Time_10_23.root");
    pp.Add("Juan", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/"
                   "20O_and_1H_to_1H_NumN_0_NumP_0_Ex0_Date_2024_10_4_Time_9_27.root");
    gSelector->SetFlag("physpluseff");
    dd.Add("phys+eff", gSelector->GetSimuFile(beam, "2H", "2H", Ex).Data());
    pp.Add("phys+eff", gSelector->GetSimuFile(beam, "1H", "1H", Ex).Data());
    gSelector->SetFlag("onlyeff");
    dd.Add("eff", gSelector->GetSimuFile(beam, "2H", "2H", Ex).Data());
    pp.Add("eff", gSelector->GetSimuFile(beam, "1H", "1H", Ex).Data());

    dd.Draw(true, "dd");
    pp.Draw(true, "pp");
}
