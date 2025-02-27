#include "TFile.h"
#include "TROOT.h"
#include "TStyle.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../Utils/AngData.cxx"
#include "../Utils/AngData.h"
#include "../Utils/PadManager.cxx"
#include "../Utils/PadManager.h"
#include "../Utils/VPlotData.h"

using Map = PubUtils::VPlotData::Labels;

void p_ang()
{
    gStyle->SetTextSize(0.065);
    gStyle->SetLabelSize(0.06, "XYZ");
    gStyle->SetTitleOffset(0.85, "Y");
    gStyle->SetTitleSize(0.07, "XYZ");

    std::vector<std::string> files {"../../Fits/pp/Outputs/sfs.root"};
    std::vector<PubUtils::AngData> data;
    for(const auto& file : files)
    {
        auto f {std::make_unique<TFile>(file.c_str())};
        auto keys {*f->Get<std::vector<std::string>>("Keys")};
        for(const auto& key : keys)
            data.emplace_back(file, key);
    }
    // Set options
    for(auto& d : data)
    {
        d.SetOpts({{"title", ""}, {"titlex", ""}, {"titley", ""}});
    }
    // First column
    PubUtils::VPlotData::Opts r0 {{"rangex", std::make_pair(16., 27.)}};
    data[0].SetOpts(r0);
    data[1].SetOpts(r0);
    // Set ndivs
    for(auto& d : data)
        d.SetNDiv("x", 507);

    // Text
    double tx {0.75};
    double ty {0.85};
    data[0].SetText(tx, ty, "g.s\\0^{+}");
    data[1].SetText(tx, ty, "E_{x} = 1.6 MeV\\2^{+}");

    PubUtils::PadManager padman {2};
    padman.SetMargins(0, -1, 0.14, 0.01, 0.10, 0.02);
    // padman.SetMargins(1, -1, 0.1, 0.01, -1, 0.00);
    padman.GetCanvas()->SetWindowSize(900, 500);
    for(int i = 0; i < 2; i++)
    {
        auto p {padman.GetPad(i)};
        p->SetLogy();
        data[i].Draw();
    }

    padman.AddXTitle(0.5, 0.06, "#theta_{CM} [#circ]", 0.525, 1);
    padman.AddYTitle(0.4, 0.03, "d#sigma / d#Omega [mb/sr]", 0.525, 0.95);

    padman.GetCanvas()->SaveAs("./Outputs/p_ang.pdf");
}
