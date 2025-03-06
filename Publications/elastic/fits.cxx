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

using Map = PubUtils::VPlotData::Labels;

void fits()
{
    // Override some gstyle opts
    gStyle->SetCanvasPreferGL(true);
    gStyle->SetAxisMaxDigits(3);

    PubUtils::PadManager padman {2};
    padman.GetCanvas()->SetWindowSize(1100, 500);

    std::vector<PubUtils::FitData> fdata;

    std::vector<std::string> files {"../../Fits/pp/Outputs/fit_juan_RPx.root",
                                    "../../Fits/dd/Outputs/fit_juan_RPx.root"};

    for(const auto& file : files)
        fdata.emplace_back(file);
    // pp
    fdata[0].SetOpts({{"title", "^{20}O(p,p)"},
                      {"rangex", std::make_pair(-2., 5.)},
                      {"rangey", std::make_pair(0., 1.5e3)},
                      {"color", PubUtils::pcol},
                      {"labels", Map {{"g0", "g.s"}, {"g1", "1st Ex"}, {"ps0", "d-breakup"}}}});
    // dd
    fdata[1].SetOpts({{"title", "^{20}O(d,d)"},
                      {"rangex", std::make_pair(-2., 18.)},
                      {"rangey", std::make_pair(0., 5e2)},
                      {"color", PubUtils::dcol},
                      {"labels", Map {{"g0", "g.s"}, {"ps0", "1n PS"}}}});

    for(int i = 0; i < fdata.size(); i++)
    {
        padman.GetPad(i)->SetLeftMargin(0.125);
        // padman.CenterXTitle();
        fdata[i].Draw();
    }
    
    // Save
    padman.GetCanvas()->SaveAs("./Outputs/elastic_xs.pdf");
}
