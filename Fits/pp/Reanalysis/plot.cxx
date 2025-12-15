#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TVirtualPad.h"

#include "AngComparator.h"
#include "PhysColors.h"

#include <map>
#include <string>
#include <vector>

void plot()
{
    // Read E.Khan data
    auto* gel {new TGraphErrors {"./elastic.dat", "%lg %lg"}};
    // auto* gin {new TGraphErrors {"./inelastic.dat", "%lg %lg"}};
    auto* gin {new TGraphErrors {"./3minus.dat", "%lg %lg"}};

    Angular::Comparator comp {"E.Khan g.s", gel};
    comp.Add("BG", "../Inputs/g0_Khan/fort.201");
    comp.Add("BG iblock=2", "../Inputs/g1_Khan/fort.201");
    comp.Fit();
    comp.Draw("", true);

    // Compare theoretical calculations for gs with CC off and on
    Angular::Comparator gs {"Ground state", nullptr};
    std::map<std::string, std::string> gss {{"iblock=0", "../Inputs/g0_Khan/fort.201"},
                                            {"iblock=2", "../Inputs/g1_Khan/fort.201"}};
    for(const auto& [key, file] : gss)
        gs.Add(key, file);
    gs.DrawTheo();
    gPad->SetLogy();

    // Same for 1st
    Angular::Comparator in {"Inelastic", nullptr};
    std::map<std::string, std::string> ins {{"iblock=0", "../Inputs/exp_Khan/fort.202"},
                                            {"iblock=2", "../Inputs/g1_Khan/fort.202"}};
    for(const auto& [key, file] : ins)
        in.Add(key, file);
    in.DrawTheo();
    gPad->SetLogy();

    // Draw both datasets in same canvas
    auto* mg {new TMultiGraph};
    mg->SetTitle("Dataset comparison;#theta_{CM} [#circ];d#sigma/d#Omega [mb/sr]");
    auto* leg {new TLegend {0.65, 0.7, 0.9, 0.9}};
    leg->SetFillStyle(0);
    leg->SetBorderSize(0);
    int idx {};
    for(auto* g : {gel, gin})
    {
        g->SetLineWidth(2);
        g->SetMarkerStyle(24);
        mg->Add(g);
        g->SetLineColor(gPhysColors->Get(2, "mpl"));
        g->SetMarkerColor(gPhysColors->Get(2, "mpl"));
        if(idx == 0)
            leg->AddEntry(g, "E. Khan", "l");
        idx++;
    }
    // Our data
    // std::vector<std::string> pp {"../Outputs/xs/g0_xs.dat", "../Outputs/xs/g1_xs.dat"};
    std::vector<std::string> pp {"../Outputs/xs/g0_xs.dat", "../Outputs/xs/g3_xs.dat"};
    idx = 0;
    for(const auto& file : pp)
    {
        auto* g {new TGraphErrors {file.c_str(), "%lg %lg %lg"}};
        g->SetLineWidth(2);
        g->SetMarkerStyle(25);
        g->SetLineColor(gPhysColors->Get(8, "mpl"));
        g->SetMarkerColor(gPhysColors->Get(8, "mpl"));
        mg->Add(g);
        if(idx == 0)
            leg->AddEntry(g, "E796", "l");
        idx++;
    }

    // Compare theoretical calculations
    Angular::Comparator theo1 {"Different E_{beam}", nullptr};
    theo1.Add("gs 43 AMeV", "../Inputs/g0_Khan/fort.201");
    theo1.Add("gs 35 AMeV", "../Inputs/g0_BG/fort.201");
    theo1.Add("1st 43 AMeV", "../Inputs/g1_Khan/fort.202");
    theo1.Add("1st 35 AMeV", "../Inputs/g1_BG/fort.202");
    theo1.DrawTheo();
    gPad->SetLogy();

    auto* c0 {new TCanvas {"c0", "E.Khan canvas"}};
    mg->SetMinimum(1);
    gPad->SetLogy();
    mg->Draw("ap");
    leg->Draw();
}
