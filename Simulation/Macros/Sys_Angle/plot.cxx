#include "TCanvas.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "Interpolators.h"

#include <iostream>
#include <string>
#include <vector>

#include "../../../Selector/Selector.h"


void plot()
{
    std::vector<double> exs {0, 6.7, 16.2};
    std::vector<std::string> labels {"gs", "6.7 MeV", "16.2 MeV"};
    std::vector<Interpolators::Efficiency> effs(exs.size());

    // Read
    int niter {1};
    for(int s = 0; s < exs.size(); s++)
    {
        auto& ex {exs[s]};
        auto& label {labels[s]};
        auto& eff {effs[s]};
        for(int i = -1; i < niter; i++)
        {
            TString tag {};
            if(i > -1)
                tag = TString::Format("sys_angle_%d", i);
            gSelector->SetTag(tag.Data());
            auto file {gSelector->GetApproxSimuFile("20O", "2H", "3H", ex)};
            eff.Add(tag.Length() ? tag.Data() : "no sys", file, "eff");
        }
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Systematic uncertainty in angle"}};
    c0->DivideSquare(exs.size());
    for(int i = 0; i < exs.size(); i++)
    {
        c0->cd(i + 1);
        effs[i].Draw(true, labels[i], gPad);
    }
}
