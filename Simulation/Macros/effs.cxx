#include "Interpolators.h"

#include "/media/Data/E796v2/Selector/Selector.h"
void effs()
{
    // Plot the effs for the current selection
    // And set also Ex
    double Ex {0}; // gs

    // Interpolators
    Interpolators::Efficiency effs;

    auto configs {gSelector->GetFlags()};
    for(const auto& config : configs)
    {
        gSelector->SetFlag(config);
        auto file {gSelector->GetSimuFile(Ex)};
        effs.Add(config, file.Data());
    }

    // Plot
    effs.Draw();
}
