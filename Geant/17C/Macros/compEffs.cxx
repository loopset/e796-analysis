#include "Interpolators.h"
#include <string>
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"

void compEffs()
{
    // Which peak
    std::string which {"eff4"};

    auto* mg {new TMultiGraph};
    // Old Si setup
    auto fold {std::make_unique<TFile>("../Outputs/17C_dp_small_si_layers.root")};
    auto* gOld {fold->Get<TGraphAsymmErrors>(which.c_str())};
    gOld->SetTitle("OldSi");

    auto fnew {std::make_unique<TFile>("../Outputs/yield_17C_d_p_255.0.root")};
    auto* gNew {fnew->Get<TGraphAsymmErrors>(which.c_str())};
    gNew->SetTitle("NewSi");

    mg->Add(gOld);
    mg->Add(gNew);

    auto* c0 {new TCanvas {"c0", "Compare effs"}};
    mg->Draw("apl plc pmc");
    gPad->BuildLegend();
}