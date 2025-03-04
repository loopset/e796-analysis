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

    std::vector<std::string> files {"../../Fits/pd/Outputs/sfs.root"};
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
        d.SetOpts({
            {"title", ""}, {"titlex", ""}, {"titley", ""},
            // {"rangex", std::make_pair(6.0, 18.)}
        });
    }
    // Y ranges
    for(auto& d : data)
        d.CenterY(1.75);

    // Line styles
    gPhysColors->Draw();
    PubUtils::VPlotData::Styles sts {
        {"l=0fnrfresco", TAttLine(gPhysColors->Get(2, "mpl"), 2, 3)},
        {"l=1fnrfresco", TAttLine(gPhysColors->Get(8, "mpl"), 1, 3)},
        {"l=2fnrfresco", TAttLine(gPhysColors->Get(18, "mpl"), 3, 3)},
        {"zr2fnr", TAttLine(gPhysColors->Get(14, "mpl"), 1, 3)},
        {"lea", TAttLine(0, 0, 0)},
    };
    for(auto& d : data)
    {
        d.SetGraphsStyle(sts);
        // d.DisableLabel("y", 1);
        d.SetNDiv("y", 507);
        d.SetNDiv("x", 505);
    }
    // Build a legend
    auto* leg {new TLegend {0.15, 0.7, 0.45, 0.95, "", "ndc"}};
    leg->SetBorderSize(0);
    leg->SetMargin(0.5);
    // leg->SetHeader("OMP", "C");
    PubUtils::AngData::GetLegendGraph(leg, sts["l=0fnrfresco"], "#DeltaL = 0");
    PubUtils::AngData::GetLegendGraph(leg, sts["l=1fnrfresco"], "#DeltaL = 1");
    PubUtils::AngData::GetLegendGraph(leg, sts["l=2fnrfresco"], "#DeltaL = 2");
    PubUtils::AngData::GetLegendGraph(leg, sts["zr2fnr"], "ZR TWOFNR");

    // Text
    double tx {0.75};
    double ty {0.8};
    data[0].SetText(tx + 0.1, ty, "g.s\\5/2^{+}");
    data[1].SetText(tx, ty, "E_{x} = 1.6 MeV\\1/2^{+}");
    data[2].SetText(tx, ty, "E_{x} = 3.1 MeV\\(1/2,3/2)^{#minus}");


    PubUtils::PadManager padman {};
    padman.SetYLow(0);
    padman.SetYUp(0);
    padman.Init(3);
    padman.SetMargins(0, -1, 0.1, 0.01, 0.125, 0.01);
    padman.GetCanvas()->SetWindowSize(1100, 500);
    for(int i = 0; i < 3; i++)
    {
        auto p {padman.GetPad(i)};
        if(i == 0)
            p->SetLeftMargin(0.13);
        padman.CenterXTitle();
        data[i].Draw();
        if(i == 0)
            leg->Draw();
    }

    padman.AddXTitle(0.25, 0.075, "#theta_{CM} [#circ]", 0.52, 0.875);
    padman.AddYTitle(0.4, 0.03, "d#sigma / d#Omega [mb/sr]", 0.525, 0.85);
    padman.GetCanvas()->SaveAs("./Outputs/pd_ang_0.pdf");
}
