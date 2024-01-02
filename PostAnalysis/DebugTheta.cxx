#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TChain.h"
#include "TROOT.h"
#include "TString.h"

#include <cstdlib>

void DebugTheta()
{
    ROOT::EnableImplicitMT();
    // Read corrected angles
    ActRoot::JoinData corr {"../configs/merger.runs"};
    // Read debug data
    TString path {"/media/Data/E796v2/RootFiles/Debug/"};
    auto* debug {new TChain {"ACTAR_Merged"}};
    for(const auto& run : corr.GetRunList())
    {
        auto name {TString::Format("Merger_Run_%04d.root", run)};
        debug->Add(path + name);
    }
    // Rename
    debug->SetName("Debug_Tree");
    // Set friends
    corr->AddFriend(debug);
    // Create RDataFrame
    ROOT::RDataFrame d {*corr.Get()};

    // Gate and only consider side events
    auto df {d.Filter(
        [](const ActRoot::MergerData& data)
        {
            bool condSize {data.fSilLayers.size() == 1};
            if(condSize)
                return data.fSilLayers.front() == "l0";
            else
                return condSize;
        },
        {"MergerData"})};

    // Define columns
    auto def {df.Define("ThetaOk", "MergerData.fThetaLight")
                  .Define("ThetaLegacy", "Debug_Tree.MergerData.fThetaLight")
                  .Define("XAxis", "MergerData.fRP.fCoordinates.fX")
                  .Define("BeamOk", "MergerData.fThetaBeam")
                  .Define("BeamLegacy", "Debug_Tree.MergerData.fThetaBeam")};
    // Difference
    def = def.Define("Diff",
                     [](float& ok, float& legacy)
                     {
                         if(ok != -1 && legacy != -1)
                             return ok - legacy;
                         else
                             return -111.f;
                     },
                     {"ThetaOk", "ThetaLegacy"});
    // Difference wrt perfect beam along X axis
    def = def.Define("DiffX", [](const ActRoot::MergerData& data) { return data.fThetaDebug - data.fThetaLight; },
                     {"MergerData"});

    // Book histograms
    auto hDiff {def.Histo2D({"hDiff", "Diff;RP.X() [mm];#theta_{Ok} - #theta_{Leg} [#circ]", 200, -10, 270, 200, -7, 7},
                            "XAxis", "Diff")};
    auto hDiffX {def.Histo2D({"hDiffX", "Diff wrt (1, 0, 0) beam;RP.X() [mm];#theta_{(1,0,0)} - #theta_{Ok} [#circ]",
                              200, -10, 270, 200, -7, 7},
                             "XAxis", "DiffX")};

    auto hBeam {
        def.Histo2D({"hBeam", "BeamOk;RP.X() [mm];#theta_{BeamOk}", 200, -10, 270, 200, -7, 7}, "XAxis", "BeamOk")};
    auto hBeamLeg {def.Histo2D({"hBeamLeg", "Beam Legacy;RP.X() [mm];#theta_{BeamLeg}", 200, -10, 270, 200, -7, 7},
                               "XAxis", "BeamLegacy")};

    // BUG: issue seems related with angle of beam-like:
    // for most events, there are unstabilitites alongZ which result in wrong fits

    // plot
    auto* c1 {new TCanvas("c1", "Diff canvas")};
    c1->DivideSquare(2);
    c1->cd(1);
    hDiff->DrawClone("colz");
    c1->cd(2);
    hDiffX->DrawClone("colz");

    auto* c2 {new TCanvas("c2", "Beam canvas")};
    c2->DivideSquare(2);
    c2->cd(1);
    hBeam->DrawClone("colz");
    c2->cd(2);
    hBeamLeg->DrawClone("colz");
}
