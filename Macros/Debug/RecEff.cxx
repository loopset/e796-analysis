#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActSilData.h"
#include "ActSilSpecs.h"
#include "ActTPCData.h"
#include "ActTypes.h"
#include "ActVoxel.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"
#include "TCanvas.h"

#include <utility>

void RecEff()
{
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EReadTPC};
    auto chain {datman.GetChain()};
    auto chain2 {datman.GetChain(ActRoot::ModeType::EReadSilMod)};
    auto chain3 {datman.GetChain(ActRoot::ModeType::EMerge)};
    chain->AddFriend(chain2.get());
    chain->AddFriend(chain3.get());
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};

    // Read specs
    auto specs {std::make_shared<ActPhysics::SilSpecs>()};
    specs->ReadFile("../../configs/detailedSilicons.conf");

    // Define region in pad plane towards left silicons
    std::pair<int, int> xrange {5, 123};
    std::pair<int, int> yrange {123, 127};

    auto isInRegion {[&](const ActRoot::Voxel& v) -> bool
                     {
                         auto& pos {v.GetPosition()};
                         auto condX {xrange.first <= pos.X() && pos.X() <= xrange.second};
                         auto condY {yrange.first <= pos.Y() && pos.Y() <= yrange.second};
                         return condX && condY;
                     }};

    ROOT::TThreadedObject<TH1D> hAll {"hAll", "Denominator", 10, 0, 10};
    ROOT::TThreadedObject<TH1D> hGated {"hGated", "Numerator", 10, 0, 10};
    ROOT::TThreadedObject<TH2D> h2d {"hCount", "Pad;Count", 10, 0, 10, 15, 0, 15};

    df.ForeachSlot(
        [&](unsigned int slot, ActRoot::TPCData& tpc, ActRoot::SilData& sil, ActRoot::ModularData& mod,
            ActRoot::MergerData& merger)
        {
            if(mod.Get("GATCONF") != 8)
                return;
            sil.ApplyFinerThresholds(specs);
            // Get side layer
            if(sil.fSiN.count("l0"))
            {
                auto& ns {sil.fSiN["l0"]};
                auto& es {sil.fSiE["l0"]};
                // Fill histograms
                auto size {ns.size()};
                if(size != 1)
                    return;
                auto& n {ns.front()};
                auto& e {es.front()};
                int count {};
                // Noise
                for(const auto& v : tpc.fRaw)
                    if(isInRegion(v))
                        count++;
                // Clusters
                for(const auto& cl : tpc.fClusters)
                    for(const auto& v : cl.GetVoxels())
                        if(isInRegion(v))
                            count++;
                // Fill
                h2d.GetAtSlot(slot)->Fill(n, count);
                hAll.GetAtSlot(slot)->Fill(n);
                if(merger.fLightIdx != -1 && count > 0)
                    hGated.GetAtSlot(slot)->Fill(n);
            }
        },
        {"TPCData", "SilData", "ModularData", "MergerData"});
    
    // Divide
    auto hEff {hGated.Merge()};
    hEff->Divide(hAll.Merge().get());

    auto*c0 {new TCanvas {"c0", "Reconstruction eff"}};
    c0->DivideSquare(4);
    c0->cd(1);
    h2d.Merge()->DrawClone("colz");
    c0->cd(2);
    hEff->DrawClone();

}
