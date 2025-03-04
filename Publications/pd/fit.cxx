#include "TCanvas.h"
#include "TPad.h"
#include "TStyle.h"
#include "TVirtualPad.h"

#include <string>
#include <vector>

#include "../Utils/Colors.h"
#include "../Utils/FitData.cxx"
#include "../Utils/FitData.h"
#include "../Utils/PadManager.cxx"
#include "../Utils/PadManager.h"

void fit()
{
    // Override some gstyle opts
    gStyle->SetCanvasPreferGL(true);
    gStyle->SetAxisMaxDigits(3);
    gStyle->SetTitleSize(0.07, "");

    PubUtils::PadManager padman {1};
    padman.GetCanvas()->SetWindowSize(700, 400);

    std::vector<PubUtils::FitData> fdata;

    std::vector<std::string> files {"../../Fits/pd/Outputs/fit_juan_RPx.root"};

    for(const auto& file : files)
        fdata.emplace_back(file);
    // pd
    fdata[0].SetOpts({{"title", "^{20}O(p,d)"},
                      {"rangex", std::make_pair(-4., 6.)},
                      // {"rangey", std::make_pair(0., 1.5e3)},
                      {"color", PubUtils::pcol},
                      {"labels", PubUtils::VPlotData::Labels {{"ps0", "(d,d) back."}}}});

    for(int i = 0; i < fdata.size(); i++)
    {
        padman.GetPad(i);
        gPad->SetLeftMargin(0.10);
        gPad->SetBottomMargin(0.12);
        // padman.CenterXTitle();
        fdata[i].Draw();
    }

    // Save
    padman.GetCanvas()->SaveAs("./Outputs/pd_xs.pdf");
}
