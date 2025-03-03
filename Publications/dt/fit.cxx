
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

    std::vector<std::string> files {"../../Fits/dt/Outputs/fit_juan_RPx.root"};

    for(const auto& file : files)
        fdata.emplace_back(file);
    // dt
    fdata[0].SetOpts({{"title", "^{20}O(d,t)"},
                      {"rangex", std::make_pair(-2., 20.)},
                      // {"rangey", std::make_pair(0., 1.5e3)},
                      {"color", PubUtils::dcol},
                      {"labels", PubUtils::VPlotData::Labels {{"ps0", "1n PS"}, {"ps1", "2n PS"}, {"ps2", {"(p,d) cont"}}}}});

    for(int i = 0; i < fdata.size(); i++)
    {
        padman.GetPad(i)->SetLeftMargin(0.125);
        // padman.CenterXTitle();
        fdata[i].Draw();
    }

    // Save
    padman.GetCanvas()->SaveAs("./Outputs/dt_xs.pdf");
}
