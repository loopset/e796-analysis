#include "TAttLine.h"
#include "TFile.h"
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

void p_ang()
{
    gStyle->SetTextSize(0.055);
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
    // Styles
    gPhysColors->Draw();
    PubUtils::VPlotData::Styles sts {
        {"ch89", TAttLine(gPhysColors->Get(6, "mpl"), 1, 3)},
        {"kd", TAttLine(gPhysColors->Get(18, "mpl"), 2, 3)},
        {"jlm", TAttLine(gPhysColors->Get(8, "mpl"), 3, 3)},
        {"ccba", TAttLine(gPhysColors->Get(14, "mpl"), 3, 3)},
    };
    for(auto& d : data)
        d.SetGraphsStyle(sts);
    // Build a legend
    auto* leg {new TLegend {0.2, 0.75, 0.5, 0.95, "", "ndc"}};
    leg->SetBorderSize(0);
    leg->SetMargin(0.5);
    leg->SetHeader("OMP", "C");
    PubUtils::AngData::GetLegendGraph(leg, sts["ch89"], "CH89");
    PubUtils::AngData::GetLegendGraph(leg, sts["kd"], "Koning-Delaroche");
    PubUtils::AngData::GetLegendGraph(leg, sts["jlm"], "E. Khan JLM");
    PubUtils::AngData::GetLegendGraph(leg, sts["ccba"], "E. Khan CCBA");

    // Text
    double tx {0.80};
    double ty {0.85};
    data[0].SetText(tx, ty, "g.s\\0^{+}");
    data[1].SetText(tx, ty, "E_{x} = 1.6 MeV\\2^{+}");

    PubUtils::PadManager padman {2};
    padman.SetMargins(0, -1, 0.14, 0.01, 0.10, 0.02);
    // padman.SetMargins(1, -1, 0.1, 0.01, -1, 0.00);
    padman.GetCanvas()->SetWindowSize(900, 450);
    for(int i = 0; i < 2; i++)
    {
        auto p {padman.GetPad(i)};
        p->SetLogy();
        data[i].Draw();
        if(i == 0)
            leg->Draw();
    }

    padman.AddXTitle(0.5, 0.06, "#theta_{CM} [#circ]", 0.525, 1);
    padman.AddYTitle(0.4, 0.03, "d#sigma / d#Omega [mb/sr]", 0.525, 0.95);

    padman.GetCanvas()->SaveAs("./Outputs/p_ang.pdf");
}
