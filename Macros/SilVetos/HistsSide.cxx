#include "ActDataManager.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TH1.h"
#include "TString.h"

#include <tuple>
#include <utility>

void HistsSide()
{
    ROOT::EnableImplicitMT();

    ActRoot::DataManager data {"../../configs/data_old.conf"};
    auto chain {data.GetJoinedData()};

    ROOT::RDataFrame df {*chain};

    // Filter side events
    auto gated {df.Filter(
        [](ActRoot::MergerData& merger)
        {
            if(merger.fSilLayers.size() == 1)
                if(merger.fSilLayers.front() == "l0")
                    return true;
            return false;
        },
        {"MergerData"})};

    // Book histograms
    int xbins {200};
    std::pair<double, double> xlims {-20, 300};
    int zbins {200};
    std::pair<double, double> zlims {-20, 300};
    auto hSP {gated.Histo2D(
        {"hSP", "SP for side;X [mm];Z [mm]", xbins, xlims.first, xlims.second, zbins, zlims.first, zlims.second},
        "fSP.fCoordinates.fX", "fSP.fCoordinates.fZ")};

    // Gate and get projections
    std::map<int, ROOT::TThreadedObject<TH1D>> pxs, pzs;
    std::vector<int> idxs {0, 1, 2, 3, 4, 5, 6, 7};
    for(const auto& idx : idxs)
    {
        pxs.emplace(std::piecewise_construct, std::forward_as_tuple(idx),
                    std::forward_as_tuple(TString::Format("px%d", idx), TString::Format("X proj %d;X [mm]", idx), xbins,
                                          xlims.first, xlims.second));
        pzs.emplace(std::piecewise_construct, std::forward_as_tuple(idx),
                    std::forward_as_tuple(TString::Format("pz%d", idx), TString::Format("Z proj %d;Z [mm]", idx), zbins,
                                          zlims.first, zlims.second));
    }
    gated.Foreach(
        [&](ActRoot::MergerData& data)
        {
            auto idx {data.fSilNs.front()};
            pxs[idx].Get()->Fill(data.fSP.X());
            pzs[idx].Get()->Fill(data.fSP.Z());
        },
        {"MergerData"});

    // plot
    auto* c1 {new TCanvas("c1", "Veto mode 3He and 4He")};
    hSP->DrawClone("colz");

    auto* cpx {new TCanvas("cpx", "X projection canvas")};
    cpx->DivideSquare(pxs.size());
    for(int i = 0; i < idxs.size(); i++)
    {
        cpx->cd(i + 1);
        pxs[idxs[i]].Merge()->DrawClone();
    }

    auto* cpz {new TCanvas("cpz", "Z projection canvas")};
    cpz->DivideSquare(pzs.size());
    for(int i = 0; i < idxs.size(); i++)
    {
        cpz->cd(i + 1);
        pzs[idxs[i]].Merge()->DrawClone();
    }
    // Write them
    auto fout {std::make_unique<TFile>("./Inputs/side_histograms.root", "recreate")};
    fout->cd();
    for(auto& [_, h] : pxs)
        h.Merge()->Write();
    for(auto& [_, h] : pzs)
        h.Merge()->Write();
}
