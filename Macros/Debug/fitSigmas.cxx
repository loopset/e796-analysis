#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <functional>
void fitSigmas()
{
    ROOT::EnableImplicitMT();

    ActRoot::DataManager dataman {"../../configs/data.conf"};
    dataman.SetRuns(155, 175);
    auto chain {dataman.GetChain()};
    auto chain2 {dataman.GetChain(ActRoot::ModeType::EFilter)};
    chain->AddFriend(chain2.get());
    ROOT::RDataFrame df {*chain};

    // Filter and define
    auto def {df.Filter("fLightIdx != -1")
                  .Define("sx", [](ActRoot::TPCData& tpc, int idx)
                          { return tpc.fClusters[idx].GetLine().GetSigmas().X(); }, {"TPCData", "fLightIdx"})
                  .Define("sy", [](ActRoot::TPCData& tpc, int idx)
                          { return tpc.fClusters[idx].GetLine().GetSigmas().Y(); }, {"TPCData", "fLightIdx"})
                  .Define("sz", [](ActRoot::TPCData& tpc, int idx)
                          { return tpc.fClusters[idx].GetLine().GetSigmas().Z(); }, {"TPCData", "fLightIdx"})
                  .Define("max",
                          [](ActRoot::TPCData& tpc, int idx)
                          {
                              auto sigmas {tpc.fClusters[idx].GetLine().GetSigmas()};
                              std::set<float, std::greater<>> set {sigmas.X(), sigmas.Y(), sigmas.Z()};
                              return *set.begin();
                          },
                          {"TPCData", "fLightIdx"})
                  .Define("min",
                          [](ActRoot::TPCData& tpc, int idx)
                          {
                              auto sigmas {tpc.fClusters[idx].GetLine().GetSigmas()};
                              std::set<float, std::greater<>> set {sigmas.X(), sigmas.Y(), sigmas.Z()};
                              return *std::next(set.begin());
                          },
                          {"TPCData", "fLightIdx"})};

    // Book
    ROOT::RDF::TH2DModel model {"hSigma", "#sigma correlations;X;Y or Z", 300, 0, 50, 300, 0, 50};
    auto hxy {def.Histo2D(model, "sx", "sy")};
    auto hxz {def.Histo2D(model, "sx", "sz")};
    auto hmm {def.Histo2D(model, "max", "min")};

    // Draw
    auto* c0 {new TCanvas {"c0", "Sigmas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hxy->SetTitle("Y correlation");
    hxy->DrawClone("colz");
    c0->cd(2);
    hxz->SetTitle("Z correlation");
    hxz->DrawClone("colz");
    c0->cd(3);
    hmm->DrawClone("colz");
}
