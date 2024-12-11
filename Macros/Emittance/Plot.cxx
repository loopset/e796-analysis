#include "ActDataManager.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDataFrame.hxx"
#include "TCanvas.h"
void Plot()
{
    // Read data
    ROOT::EnableImplicitMT();
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EFilter};
    auto chain {datman.GetChain()};
    ROOT::RDataFrame df {*chain};

    // Filter by only one BL cluster in event
    auto def {df.Filter(
                    [](ActRoot::TPCData& d)
                    {
                        if(d.fClusters.size() == 1)
                            if(d.fClusters.front().GetIsBeamLike())
                                return true;
                        return false;
                    },
                    {"TPCData"})
                  .Define("AtBegin", [](ActRoot::TPCData& d) { return d.fClusters.front().GetLine().MoveToX(0); },
                          {"TPCData"})
                  .Define("AtEnd", [](ActRoot::TPCData& d) { return d.fClusters.front().GetLine().MoveToX(126); },
                          {"TPCData"})};

    // Book histograms
    ROOT::RDF::TH2DModel mEmittance {"hEmitt", "Emittance;Y [pad];Z [btb]", 200, 0, 128, 200, 0, 128};
    auto hBegin {def.Define("y", "AtBegin.Y()").Define("z", "AtBegin.Z()").Histo2D(mEmittance, "y", "z")};

    // Plot
    auto* c0 {new TCanvas {"c0", "Emittance canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hBegin->DrawClone("colz");
}
