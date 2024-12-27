#include "ActDataManager.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TH1.h"
#include "TString.h"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
void DoHists(TString mode)
{
    mode.ToLower();
    std::cout << "Running for : " << mode << '\n';
    // Assign things depending on mode
    bool isSide {};
    std::vector<int> idxs;
    std::function<bool(ActRoot::MergerData&)> filter;
    if(mode == "side")
    {
        isSide = true;
        idxs = {0, 1, 2, 3, 4, 5, 6, 7};
        filter = [](ActRoot::MergerData& merger)
        {
            if(merger.fSilLayers.size() == 1)
                if(merger.fSilLayers.front() == "l0")
                    return true;
            return false;
        };
    }
    else if(mode == "antiveto")
    {
        idxs = {0, 2, 3, 4, 5, 7, 8, 10};
        filter = [](ActRoot::MergerData& merger)
        {
            auto& siL {merger.fSilLayers};
            auto& siN {merger.fSilNs};
            // Contains energy in F0
            bool isInF0 {std::find(siL.begin(), siL.end(), "f0") != siL.end()};
            // Contains energy in F1
            bool isInF1 {std::find(siL.begin(), siL.end(), "f1") != siL.end()};
            // Force coincidence of index in 0 and 1
            bool shareIndex {};
            if(isInF0 && isInF1)
            {
                shareIndex = (siN[0] == siN[1]);
            }
            return isInF0 && isInF1 && shareIndex;
        };
    }
    else if(mode == "veto")
    {
        idxs = {0, 2, 3, 4, 5, 7, 8, 10};
        filter = [](const ActRoot::MergerData& d)
        {
            auto condSize {d.fSilLayers.size() >= 1};
            if(condSize)
                return (d.fSilLayers.front() == "f0");
            else
                return false;
        };
    }
    else
        throw std::invalid_argument("DoHists: no known mode " + mode);

    // Read data
    ROOT::EnableImplicitMT();
    ActRoot::DataManager datman {"../../configs/data.conf"};
    auto chain {datman.GetChain()};
    ROOT::RDataFrame df {*chain};

    // Filter
    auto gated {df.Filter(filter, {"MergerData"})};

    // Book histograms
    int xybins {200};
    std::pair<double, double> xlims {-20, 300};
    int zbins {200};
    std::pair<double, double> zlims {-20, 300};
    auto hSP {gated.Histo2D(
        {"hSP", "SP;X | Y [mm];Z [mm]", xybins, xlims.first, xlims.second, zbins, zlims.first, zlims.second},
        isSide ? "fSP.fCoordinates.fX" : "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Gate and get projections
    std::map<int, ROOT::TThreadedObject<TH1D>> pxys, pzs;
    for(const auto& idx : idxs)
    {
        pxys.emplace(std::piecewise_construct, std::forward_as_tuple(idx),
                     std::forward_as_tuple(TString::Format("pxy%d", idx),
                                           TString::Format("X or Y proj %d;X | Y [mm]", idx), xybins, xlims.first,
                                           xlims.second));
        pzs.emplace(std::piecewise_construct, std::forward_as_tuple(idx),
                    std::forward_as_tuple(TString::Format("pz%d", idx), TString::Format("Z proj %d;Z [mm]", idx), zbins,
                                          zlims.first, zlims.second));
    }
    gated.ForeachSlot(
        [&](unsigned int slot, ActRoot::MergerData& data)
        {
            auto idx {data.fSilNs.front()};
            if(pxys.count(idx))
            {
                pxys[idx].GetAtSlot(slot)->Fill(isSide ? data.fSP.X() : data.fSP.Y());
                pzs[idx].GetAtSlot(slot)->Fill(data.fSP.Z());
            }
        },
        {"MergerData"});
    // Merge
    for(auto map : {&pxys, &pzs})
        for(auto& [_, h] : *map)
            h.Merge();

    // plot
    auto* c1 {new TCanvas("c1", ("Hists canvas for " + mode).Data())};
    hSP->DrawClone("colz");

    auto* cpxy {new TCanvas("cpxy", "X | Y projection canvas")};
    cpxy->DivideSquare(pxys.size());
    int pad {0};
    for(auto& [_, p] : pxys)
    {
        cpxy->cd(pad + 1);
        p.GetAtSlot(0)->DrawClone();
        pad++;
    }
    auto* cpz {new TCanvas("cpz", "Z projection canvas")};
    cpz->DivideSquare(pzs.size());
    pad = 0;
    for(auto& [_, p] : pzs)
    {
        cpz->cd(pad + 1);
        p.GetAtSlot(0)->DrawClone();
        pad++;
    }
    // Write them
    auto fout {std::make_unique<TFile>(TString::Format("./Inputs/%s_histograms.root", mode.Data()), "recreate")};
    fout->cd();
    for(auto& [_, h] : pxys)
        h.GetAtSlot(0)->Write();
    for(auto& [_, h] : pzs)
        h.GetAtSlot(0)->Write();
}
