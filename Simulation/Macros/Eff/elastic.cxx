#include "Interpolators.h"

#include <string>

#include "../../../Selector/Selector.h"

void elastic()
{
    std::string beam {"20O"};
    std::string target {"2H"};
    std::string light {"2H"};
    double Ex {0};

    Interpolators::Efficiency effs;
    effs.Add("Juan", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_2H_NumN_0_NumP_0_Ex0_Date_2024_10_2_Time_10_23.root");
    gSelector->SetFlag("physpluseff");
    effs.Add("both", gSelector->GetSimuFile(beam, target, light, Ex).Data());
    gSelector->SetFlag("onlyeff");
    effs.Add("eff", gSelector->GetSimuFile(beam, target, light, Ex).Data());
    
    effs.Draw();
}
