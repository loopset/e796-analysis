#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActSilData.h"
#include "ActSilSpecs.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TVirtualPad.h"

#include <memory>
#include <vector>

void DebugSide()
{
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EReadSilMod};
    datman.SetRuns(155, 225);
    auto chain {datman.GetChain()};
    auto chain2 {datman.GetChain(ActRoot::ModeType::EMerge)};
    auto chain3 {datman.GetChain(ActRoot::ModeType::EFilter)};
    chain->AddFriend(chain2.get());
    chain->AddFriend(chain3.get());
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};

    // Read specs
    auto specs {std::make_shared<ActPhysics::SilSpecs>()};
    specs->ReadFile("../../configs/detailedSilicons.conf");

    // Book things
    ROOT::TThreadedObject<TH1D> hMult {"hMult", "Mult after threshold;Pad;Counts", 10, 0, 10};
    ROOT::TThreadedObject<TH2D> hEs {"hEs", "E per pad;Pad;Energy [MeV]", 10, 0, 10, 400, 0, 40};
    ROOT::TThreadedObject<TH2D> hEsGated {"hEs", "E per pad gated with rec track;Pad;Energy [MeV]", 10, 0, 10, 200, 0,
                                          40};
    ROOT::TThreadedObject<TH2D> hCoin {"hCoin", "Coincidences;Pad;Coincidences mult", 10, 0, 10, 10, 0, 10};
    // Success of reconstruction
    ROOT::TThreadedObject<TH1D> hCount {"hCount", "Mult == 1;Pad;Counts", 10, 0, 10};
    ROOT::TThreadedObject<TH1D> hOk {"hOk", "Mult == 1 ok rec;Pad;Counts", 10, 0, 10};
    ROOT::TThreadedObject<TH1D> hMatch {"hMatch", "Match not ok;Pad;Counts", 10, 0, 10};

    df.ForeachSlot(
        [&](unsigned int slot, ActRoot::SilData& sil, ActRoot::MergerData& m, ActRoot::ModularData& mod,
            ActRoot::TPCData& tpc)
        {
            if(mod.Get("GATCONF") != 8) // 8 == side layer
                return;
            // Apply thresholds
            sil.ApplyFinerThresholds(specs);
            // Get side layer
            if(sil.fSiN.count("l0"))
            {
                auto& ns {sil.fSiN["l0"]};
                auto& es {sil.fSiE["l0"]};
                // Fill histograms
                auto size {ns.size()};
                auto ntracks {tpc.fClusters.size()};
                for(int i = 0; i < size; i++)
                {
                    auto& n {ns[i]};
                    auto& e {es[i]};
                    hMult.GetAtSlot(slot)->Fill(n);
                    hEs.GetAtSlot(slot)->Fill(n, e);
                    hCoin.GetAtSlot(slot)->Fill(n, size);
                    if(m.fLightIdx != -1)
                        hEsGated.GetAtSlot(slot)->Fill(n, e);
                    // Mult == 1 and n of filtered tracks == 2 or 3
                    if(size == 1 && (ntracks == 2 || ntracks == 3) && tpc.fRPs.size())
                    {
                        hCount.GetAtSlot(slot)->Fill(n);
                        if(m.fLightIdx != -1)
                            hOk.GetAtSlot(slot)->Fill(n);
                        if(m.fFlag == "SP not matched")
                            hMatch.GetAtSlot(slot)->Fill(n);
                    }
                }
            }
        },
        {"SilData", "MergerData", "ModularData", "TPCData"});

    // Divide
    auto hEff {hOk.Merge()};
    hEff->Divide(hCount.Merge().get());

    // Draw
    auto* c0 {new TCanvas {"c0", "Debug side canvas"}};
    c0->DivideSquare(6);
    c0->cd(1);
    gPad->SetLogy();
    hMult.Merge()->DrawClone();
    c0->cd(2);
    hEs.Merge()->DrawClone("colz");
    c0->cd(3);
    hEsGated.Merge()->DrawClone("colz");
    c0->cd(4);
    hCoin.Merge()->DrawClone("colz");
    c0->cd(5);
    hEff->DrawClone();
    c0->cd(6);
    hMatch.Merge()->DrawClone();
    // specs->GetLayer("l0").GetSilMatrix()->DrawClone(false);
}
