#include "ActDataManager.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TH1.h"
#include "TROOT.h"
#include "TString.h"

#include <utility>
#include <vector>


void HistsVeto()
{
    ROOT::EnableImplicitMT();
    ActRoot::DataManager datman {"./../../configs/data.conf"};
    auto chain {datman.GetJoinedData()};
    ROOT::RDataFrame d {*chain};

    // VETO using only ESi0 > 0, suitable for
    // 3He and 4He reactions
    auto df {d.Filter(
        [](const ActRoot::MergerData& d)
        {
            auto condSize {d.fSilLayers.size() >= 1};
            if(condSize)
                return (d.fSilLayers.front() == "f0");
            else
                return false;
        },
        {"MergerData"})};

    // Book histograms
    int ybins {200};
    std::pair<double, double> ylims {-20, 300};
    int zbins {200};
    std::pair<double, double> zlims {-20, 300};
    auto hSP {df.Histo2D(
        {"hSP", "SP with E0 > 0;Y [mm];Z [mm]", ybins, ylims.first, ylims.second, zbins, zlims.first, zlims.second},
        "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Gate and get projections
    std::vector<int> idxs {0, 2, 3, 4, 5, 7, 8, 10};
    std::map<int, ROOT::TThreadedObject<TH1D>> pys, pzs;
    auto* pmodel {new TH1D {"pmodel", "Model", 250, -20, 300}};
    for(const auto& i : idxs)
    {
        pys.emplace(i, *pmodel);
        pys[i].Get()->SetNameTitle(TString::Format("py%d", i), TString::Format("Y proj for %d;Y [mm]", i));
        pzs.emplace(i, *pmodel);
        pzs[i].Get()->SetNameTitle(TString::Format("pz%d", i), TString::Format("Z proj for %d;Z [mm]", i));
    }
    df.Foreach(
        [&](const ActRoot::MergerData& d)
        {
            auto n {d.fSilNs.front()};
            if(pys.count(n))
            {
                pys[n].Get()->Fill(d.fSP.Y());
                pzs[n].Get()->Fill(d.fSP.Z());
            }
        },
        {"MergerData"});

    // plot
    auto* c1 {new TCanvas("c1", "Veto mode 3He and 4He")};
    hSP->DrawClone("colz");

    auto* cpy {new TCanvas("cpy", "Y projection canvas")};
    cpy->DivideSquare(pys.size());
    int pad {0};
    for(auto& [_, p] : pys)
    {
        cpy->cd(pad + 1);
        p.Merge()->DrawClone();
        pad++;
    }

    auto* cpz {new TCanvas("cpz", "Z projection canvas")};
    cpz->DivideSquare(pzs.size());
    pad = 0;
    for(auto& [_, p] : pzs)
    {
        cpz->cd(pad + 1);
        p.Merge()->DrawClone();
        pad++;
    }

    // Write them
    auto fout {std::make_unique<TFile>("./Inputs/veto_histograms.root", "recreate")};
    fout->cd();
    for(auto& [_, h] : pys)
        h->Write();
    for(auto& [_, h] : pzs)
        h->Write();
}
