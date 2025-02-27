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

void d_ang()
{
    gStyle->SetTextSize(0.065);
    gStyle->SetLabelSize(0.06, "XYZ");
    gStyle->SetTitleOffset(0.85, "Y");
    gStyle->SetTitleSize(0.07, "XYZ");

    std::vector<std::string> files {"../../Fits/dd/Outputs/sfs.root"};
    std::vector<PubUtils::AngData> data;
    for(const auto& file : files)
    {
        auto f {std::make_unique<TFile>(file.c_str())};
        auto keys {*f->Get<std::vector<std::string>>("Keys")};
        for(const auto& key : keys)
        {
            data.emplace_back(file, key);
        }
    }
    // Set options
    for(auto& d : data)
    {
        d.SetOpts({{"title", ""}, {"titlex", ""}, {"titley", ""}});
    }
    // First column
    PubUtils::VPlotData::Opts c0 {{"rangex", std::make_pair(14., 24.)}};
    PubUtils::VPlotData::Opts c1 {{"rangex", std::make_pair(16., 24.)}};
    data[0].SetOpts(c0);
    data[2].SetOpts(c0);
    PubUtils::VPlotData::Opts r0 {{"labelx", false}};
    data[0].SetOpts(r0);
    data[1].SetOpts(r0);
    // Set ndivs
    for(auto& d : data)
        d.SetNDiv("x", 507);
    data[1].DisableLabel("y", 1);
    data[2].DisableLabel("x", -1);

    // Text
    double tx {0.8};
    double ty {0.85};
    data[0].SetText(tx, ty, "g.s\\0^{+}");
    data[1].SetText(tx, ty, "E_{x} = 1.6 MeV\\2^{+}");
    data[2].SetText(tx, ty, "E_{x} = 4 MeV\\2^{+}");
    data[3].SetText(tx, ty, "E_{x} = 5.6 MeV\\3^{-}");

    PubUtils::PadManager padman {};
    padman.SetYLow(0);
    padman.SetYUp(0);
    padman.Init(4);
    padman.SetMargins(0, -1, 0.1, 0.01, 0.01, 0.01);
    padman.SetMargins(1, -1, 0.1, 0.01, -1, 0.00);
    padman.GetCanvas()->SetWindowSize(900, 700);
    for(int i = 0; i < 4; i++)
    {
        auto p {padman.GetPad(i)};
        // p->SetTopMargin(0.05);
        // p->SetLeftMargin(0.12);
        // p->SetBottomMargin(0.16);
        if(i == 0)
            p->SetLogy();
        padman.CenterXTitle();
        data[i].Draw();
    }

    padman.AddXTitle(0.2, 0.05, "#theta_{CM} [#circ]", 0.525);
    padman.AddYTitle(0.4, 0.03, "d#sigma / d#Omega [mb/sr]", 0.525, 0.85);

    padman.GetCanvas()->SaveAs("./Outputs/d_ang.pdf");
}
