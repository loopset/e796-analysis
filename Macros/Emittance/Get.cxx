#include "ActDataManager.h"
#include "ActInputParser.h"
#include "ActLine.h"
#include "ActModularData.h"
#include "ActTPCData.h"
#include "ActTypes.h"
#include "ActVoxel.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDFHelpers.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "Rtypes.h"

#include <iostream>
#include <vector>

typedef ROOT::RVecF Vector;

void Get()
{
    // Read data
    ROOT::EnableImplicitMT();
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EFilter};
    auto chain {datman.GetChain()};
    auto chain2 {datman.GetChain(ActRoot::ModeType::EMerge)};
    auto chain3 {datman.GetChain(ActRoot::ModeType::EReadSilMod)};
    chain->AddFriend(chain2.get());
    chain->AddFriend(chain3.get());
    chain->SetBranchStatus("fClusters.fVoxels", false);
    ROOT::RDataFrame df {*chain};

    // Read conversion factors
    float padSide {2}; // mm
    ActRoot::InputParser parser {"../../configs/detector.conf"};
    auto merger {parser.GetBlock("Merger")};
    float driftFactor {static_cast<float>(merger->GetDouble("DriftFactor"))};
    std::cout << "-> DriftFactor : " << driftFactor << '\n';

    // Filter by only one BL cluster in event
    auto def {df.Filter([](ActRoot::ModularData& m) { return m.Get("GATCONF") == 1; }, {"ModularData"})
                  .Filter("fClusters.fIsBeamLike.size() == 1")
                  .Filter("fClusters.fIsBeamLike.front() == true")
                  .Filter(
                      [](const Vector& first, const Vector& second)
                      {
                          bool condA {first.front() <= 0};
                          bool condB {second.front() >= 127};
                          return condA && condB;
                      },
                      {"fClusters.fXRange.first", "fClusters.fXRange.second"})
                  .Define("Line",
                          [&](ActRoot::TPCData& data)
                          {
                              auto& voxels {data.fClusters.front().GetVoxels()};
                              ActPhysics::Line line;
                              line.FitVoxels(voxels, true, true, true);
                              line.Scale(padSide, driftFactor);
                              return line;
                          },
                          {"TPCData"})
                  .Define("AtBegin", [](const ActPhysics::Line& l) { return l.MoveToX(0); }, {"Line"})
                  .Define("AtEnd", [](const ActPhysics::Line& l) { return l.MoveToX(256); }, {"Line"})};
    auto count {def.Count()};
    ROOT::RDF::Experimental::AddProgressBar(def);
    def.Snapshot("Emittance_Tree", "./Outputs/emittance.root", {"AtBegin", "AtEnd", "Line", "fRun", "fEntry"});
    std::cout << "Processed events : " << *count << '\n';
}
