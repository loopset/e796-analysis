#include "ActDataManager.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"

#include <vector>

void beamSP()
{
    ROOT::EnableImplicitMT();

    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EReadTPC};
    auto chain {datman.GetJoinedData()};
    ROOT::RDataFrame df {*chain};

    auto gated {df.Filter("fClusters.size() == 1")
                    .Define("SPy", [](ActRoot::TPCData& d) { return d.fClusters.front().GetLine().MoveToX(181).Y(); },
                            {"TPCData"})
                    .Define("SPz", [](ActRoot::TPCData& d) { return d.fClusters.front().GetLine().MoveToX(181).Z(); },
                            {"TPCData"})};
    auto hSP {gated.Histo2D({"hBeam", "Beam point at F0;Y [pad];Z [btb]", 350, 0, 128, 350, 0, 128}, "SPy", "SPz")};

    // Draw
    auto* c0 {new TCanvas {"c0", "Beam spot ID"}};
    hSP->DrawClone("colz");
}
