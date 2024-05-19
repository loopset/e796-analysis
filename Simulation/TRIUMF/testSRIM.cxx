#include "ActRunner.h"
#include "ActSRIM.h"

#include "TCanvas.h"
#include "TGraph.h"

#include "GuiTypes.h"

#include <string>
void testSRIM()
{
    ActPhysics::SRIM srim {"sil", "../../Calibrations/SRIMData/raw/1H_silicon.txt"};
    ActSim::Runner runner {&srim, nullptr, nullptr, 0};
    std::string silString {"sil"};
    double silWidth {1.5}; // mm

    auto* g {new TGraph};
    auto* gg {new TGraph};
    auto* gel {new TGraph};
    auto* gin {new TGraph};
    auto* gout {new TGraph};
    auto* gother {new TGraph};

    for(double T3In = 5; T3In < 40; T3In += 5)
    {
        double eLoss {};
        double T3AfterSil0 {};
        // initial range
        auto RIni {srim.EvalDirect(silString, T3In)};
        // RLeft without straggling
        auto RLeft {RIni - silWidth};
        if(RLeft < 0) // particle stopped in silicon
        {
            eLoss = T3In;
        }
        else
        {
            auto RLeftWithStraggling = RLeft; // no straggling

            T3AfterSil0 = T3In - srim.EvalInverse(silString, RLeft);

            eLoss = {T3In - T3AfterSil0};
        }
        auto other {srim.Slow(silString, T3In, silWidth)};
    //     auto RIni {EvalDirect(material, Tini)};
    // // Compute distance
    // auto dist {thickness / TMath::Cos(angleInRad)};
    // // New range
    // auto RAfter {RIni - dist};
    // return EvalInverse(material, RAfter);


        // Fill graphs
        g->SetPoint(g->GetN(), T3In, RIni);
        gg->SetPoint(gg->GetN(), T3In, RLeft);
        gel->SetPoint(gel->GetN(), T3In, srim.EvalStoppingPower(silString, T3In));
        gin->SetPoint(gin->GetN(), T3In, T3In);
        gout->SetPoint(gout->GetN(), T3In, T3AfterSil0);
        gother->SetPoint(gother->GetN(), T3In, other);
    }

    auto* c0 {new TCanvas {"c0"}};
    c0->DivideSquare(4);
    c0->cd(1);
    g->Draw("al");
    c0->cd(2);
    gg->Draw("al");
    c0->cd(3);
    gel->Draw("al");
    c0->cd(4);
    gin->Draw("al");
    gout->Draw("l");
    gother->Draw("l");
}
