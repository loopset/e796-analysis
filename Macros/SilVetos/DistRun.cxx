#include "ActDataManager.h"
#include "ActLine.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TFile.h"
#include "TH1.h"
#include "TString.h"

#include "Math/Point3Dfwd.h"

#include <memory>
#include <tuple>
#include <utility>
#include <vector>

void DistRun()
{
    ROOT::EnableImplicitMT();

    ActRoot::DataManager data {"../../configs/data.conf"};
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

    // Define distances in mm
    double base {256.};
    std::vector<double> dists;
    for(double d = 85; d < 115; d += 2)
        dists.push_back(base + d);

    int xbins {200};
    std::pair<double, double> xlims {-20, 300};
    int zbins {200};
    std::pair<double, double> zlims {-20, 300};
    // Save
    auto f {std::make_unique<TFile>("./Outputs/Dists/histos.root", "recreate")};
    f->WriteObject(&dists, "dists");
    for(const auto& dist : dists)
    {
        std::cout << "Distance : " << dist << '\n';
        // Redefine
        auto node {gated.Define("NewSP",
                                [&, dist](ActRoot::MergerData& d)
                                {
                                    // Reconstruct line from BP and SP
                                    auto p {d.fBP};
                                    auto dir {(d.fSP - d.fBP)};
                                    ActRoot::Line line {p, dir, 0};
                                    return line.MoveToY(dist);
                                },
                                {"MergerData"})};
        // Fill histograms!
        ROOT::TThreadedObject<TH2D> hSP {ROOT::TNumSlots {node.GetNSlots()},
                                         "hSP",
                                         TString::Format("Side %.2f mm;X [mm];Z [mm]", dist),
                                         xbins,
                                         xlims.first,
                                         xlims.second,
                                         zbins,
                                         zlims.first,
                                         zlims.second};
        std::map<int, ROOT::TThreadedObject<TH1D>> pxs, pzs;
        std::vector<int> idxs {0, 1, 2, 3, 4, 5, 6, 7};
        for(const auto& idx : idxs)
        {
            pxs.emplace(std::piecewise_construct, std::forward_as_tuple(idx),
                        std::forward_as_tuple(ROOT::TNumSlots {node.GetNSlots()}, TString::Format("px%d", idx),
                                              TString::Format("%.2f mm X proj %d;X [mm]", dist, idx), xbins,
                                              xlims.first, xlims.second));
            pzs.emplace(std::piecewise_construct, std::forward_as_tuple(idx),
                        std::forward_as_tuple(ROOT::TNumSlots {node.GetNSlots()}, TString::Format("pz%d", idx),
                                              TString::Format("%.2f mm Z proj %d;Z [mm]", dist, idx), zbins,
                                              zlims.first, zlims.second));
        }
        node.ForeachSlot(
            [&](unsigned int slot, ActRoot::MergerData& data, ROOT::Math::XYZPointF& sp)
            {
                hSP.GetAtSlot(slot)->Fill(sp.X(), sp.Z());
                auto idx {data.fSilNs.front()};
                if(pxs.count(idx))
                {
                    pxs[idx].GetAtSlot(slot)->Fill(sp.X());
                    pzs[idx].GetAtSlot(slot)->Fill(sp.Z());
                }
            },
            {"MergerData", "NewSP"});

        // Write data
        f->cd();
        auto path {TString::Format("d_%.1f_mm/", dist)};
        auto* dir {f->mkdir(path)};
        dir->cd();
        hSP.Merge()->Write();
        for(auto m : {&pxs, &pzs})
            for(auto& [_, h] : *m)
                h.Merge()->Write();
        f->cd();
    }
}
