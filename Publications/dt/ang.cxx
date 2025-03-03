#include "TCanvas.h"
#include "TFile.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"

#include "PhysColors.h"

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

void ang()
{
    gStyle->SetTextSize(0.065);
    gStyle->SetLabelSize(0.06, "XYZ");
    gStyle->SetTitleOffset(0.85, "Y");
    gStyle->SetTitleSize(0.07, "XYZ");
    gStyle->SetLegendTextSize(0.07);

    std::vector<std::string> files {"../../Fits/dt/Outputs/sfs.root"};
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
        d.SetOpts({{"title", ""}, {"titlex", ""}, {"titley", ""}, {"rangex", std::make_pair(5.0, 15.)}});
    }
    // Y ranges
    for(auto& d : data)
        d.CenterY(1.75);

    // Line styles
    gPhysColors->Draw();
    PubUtils::VPlotData::Styles sts {
        {"l=0", TAttLine(gPhysColors->Get(2, "mpl"), 2, 3)},
        {"l=1", TAttLine(gPhysColors->Get(8, "mpl"), 1, 3)},
        {"l=2", TAttLine(gPhysColors->Get(18, "mpl"), 3, 3)},
    };
    for(auto& d : data)
    {
        d.SetGraphsStyle(sts);
        d.DisableLabel("y", 1);
        d.SetNDiv("y", 507);
    }
    // Build a legend
    auto* leg {new TLegend {0.2, 0.7, 0.5, 0.95, "", "ndc"}};
    leg->SetBorderSize(0);
    leg->SetMargin(0.5);
    // leg->SetHeader("OMP", "C");
    PubUtils::AngData::GetLegendGraph(leg, sts["l=0"], "#DeltaL = 0");
    PubUtils::AngData::GetLegendGraph(leg, sts["l=1"], "#DeltaL = 1");
    PubUtils::AngData::GetLegendGraph(leg, sts["l=2"], "#DeltaL = 2");

    // Text
    double tx {0.8};
    double ty {0.8};
    data[0].SetText(tx, ty, "g.s\\5/2^{+}");
    data[1].SetText(tx, ty, "E_{x} = 1.6 MeV\\1/2^{+}");
    data[2].SetText(tx, ty, "E_{x} = 3.1 MeV\\(1/2,3/2)^{#minus}");
    data[3].SetText(tx, ty, "E_{x} = 4.6 MeV\\(1/2,3/2)^{#minus}");
    data[4].SetText(tx, ty, "E_{x} = 6.6 MeV\\(1/2,3/2)^{#minus}");
    data[5].SetText(tx, ty, "E_{x} = 7.9 MeV\\(1/2,3/2)^{#minus}");
    data[6].SetText(tx, ty, "E_{x} = 8.9 MeV\\(1/2,3/2)^{#minus}");
    data[7].SetText(tx, ty, "E_{x} = 10.7 MeV\\(1/2,3/2)^{#minus}");
    data[8].SetText(tx, ty, "E_{x} = 14.9 MeV\\Not sure");


    PubUtils::PadManager padman {};
    padman.SetYLow(0);
    padman.SetYUp(0);
    padman.Init(4);
    padman.SetMargins(0, -1, 0.1, 0.01, 0.01, 0.01);
    padman.SetMargins(1, -1, 0.1, 0.01, -1, 0.00);
    padman.GetCanvas()->SetWindowSize(800, 500);
    for(int i = 0; i < 4; i++)
    {
        auto p {padman.GetPad(i)};
        padman.CenterXTitle();
        data[i].Draw();
        if(i == 0)
            leg->Draw();
    }

    padman.AddXTitle(0.2, 0.05, "#theta_{CM} [#circ]", 0.52, 0.875);
    padman.AddYTitle(0.4, 0.03, "d#sigma / d#Omega [mb/sr]", 0.525, 0.85);
    padman.GetCanvas()->SaveAs("./Outputs/ang_0.pdf");

    PubUtils::PadManager padman1 {};
    padman1.SetYLow(0);
    padman1.SetYUp(0);
    padman1.Init(6);
    padman1.SetMargins(0, -1, 0.12, 0.01, 0.01, 0.01);
    padman1.SetMargins(1, -1, 0.12, 0.01, 0.01, 0.01);
    padman1.SetMargins(2, -1, 0.12, 0.01, -1, 0.01);
    padman1.GetCanvas()->SetWindowSize(600, 800);
    for(int i = 0; i < 5; i++)
    {
        auto p {padman1.GetPad(i)};
        if(i == 3)
            p->SetBottomMargin(0.075);
        padman1.CenterXTitle();
        data[i + 4].Draw();
        if(i == 0)
            leg->Draw();
    }

    padman1.AddXTitle(0.2, 0.05, "#theta_{CM} [#circ]", 0.57, 0.75);
    padman1.AddYTitle(0.4, 0.035, "d#sigma / d#Omega [mb/sr]", 0.525, 0.9);
    padman1.GetCanvas()->SaveAs("./Outputs/ang_1.pdf");
}
