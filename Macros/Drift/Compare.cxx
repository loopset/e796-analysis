#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"
#include "TMultiGraph.h"
#include "TProfile.h"
#include "TString.h"
#include "TVirtualPad.h"

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../../Selector/Selector.h"
void Compare()
{
    // List values
    std::vector<double> drifts {2.344};
    std::map<std::string, std::string> names {{"pd", "1H_2H"}, {"dt", "2H_3H"}};

    // Read data
    std::vector<TH2D*> hs2d;
    std::vector<TProfile*> hsp;
    std::vector<TGraph*> gs;
    auto* mg {new TMultiGraph};
    for(const auto& drift : drifts)
    {
        for(const auto& [key, name] : names)
        {
            auto fullname {"./Outputs/hs_20O_" + name + "_" + gSelector->GetFlag().c_str() + "_drift_" +
                           TString::Format("%.4f", drift).Data() + ".root"};
            auto file {std::make_unique<TFile>(fullname.c_str())};
            auto* h2d {file->Get<TH2D>("hExRPZ")};
            h2d->SetTitle(("2D for " + key).c_str());
            auto* hp {file->Get<TProfile>("hProfX")};
            hp->SetTitle(("Prof for " + key).c_str());
            auto* func {hp->GetFunction("pol2")};
            auto* g {new TGraph {func}};
            g->SetTitle(key.c_str());
            g->SetLineWidth(2);
            h2d->SetDirectory(nullptr);
            hp->SetDirectory(nullptr);
            hs2d.push_back(h2d);
            hsp.push_back(hp);
            gs.push_back(g);
            mg->Add(g);
        }
    }

    // Plot
    auto* c0 {new TCanvas {"c0", "Drift correction comparison"}};
    c0->DivideSquare(4);
    c0->cd(1);
    mg->SetTitle("Drift correction;SP.Z() [mm];E_{gs} [MeV]");
    mg->Draw("al plc");
    gPad->BuildLegend();

    auto* c1 {new TCanvas {"c1", "Profiles canvas"}};
    c1->DivideSquare(hsp.size());
    for(int i = 0; i < hsp.size(); i++)
    {
        c1->cd(i + 1);
        hsp[i]->Draw();
    }
}
