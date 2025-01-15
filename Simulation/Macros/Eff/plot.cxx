#include "Interpolators.h"

#include <string>

#include "../../../Selector/Selector.h"
void plot()
{
    std::string target {"2H"};
    std::string light {"2H"};

    Interpolators::Efficiency effs;
    auto files {gSelector->GetSimuFiles("20O", target, light)};
    for(int i = 0; i < files.size(); i++)
        effs.Add(std::to_string(i), files[i]);
    effs.Draw();
}
