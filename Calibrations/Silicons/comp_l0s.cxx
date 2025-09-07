#include "ActCalibrationManager.h"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TVirtualPad.h"

#include <string>

void comp_l0s()
{
    ActRoot::CalibrationManager calman;
    calman.ReadCalibration("./Outputs/thesis_l0.dat");

    ActRoot::CalibrationManager calold;
    calold.ReadCalibration("./SiL.cal");

    int nsil {8};
    std::vector<TMultiGraph*> mgs;
    for(int s = 0; s < nsil; s++)
    {
        std::string key {"Sil_l0_" + std::to_string(s) + "_E"};
        auto* mg {new TMultiGraph};
        mg->SetTitle(TString::Format("L0_%d;Channel;E [MeV]", s));
        int idx {};
        for(auto cal : {&calman, &calold})
        {
            auto* g {new TGraphErrors};
            for(int channel = 0; channel < 16384; channel += 4000)
                g->AddPoint(channel, cal->ApplyCalibration(key, channel));
            // Set style
            g->SetLineWidth(2);
            if(idx > 0)
            {
                g->SetLineStyle(2);
                g->SetLineColor(46);
            }
            mg->Add(g);
            idx++;
        }
        mgs.push_back(mg);
    }

    auto* c0 {new TCanvas {"c0", "Comparison of cals"}};
    c0->DivideSquare(mgs.size());
    for(int i = 0; i < mgs.size(); i++)
    {
        c0->cd(i + 1);
        mgs[i]->Draw("al");
    }
}
