#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"
#include "ActSilMatrix.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <fstream>
#include <iostream>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
void DebugHes()
{
    // Enable MT
    ROOT::EnableImplicitMT();

    // Get data
    ActRoot::JoinData data {"../configs/merger.runs", "../configs/corrections.runs"};
    ROOT::RDataFrame d {*data.Get()};

    // Gate on VETO events
    auto df {d.Filter(
        [](const ActRoot::MergerData& d)
        {
            auto condSize {d.fSilLayers.size() >= 1};
            if(condSize)
                return (d.fSilLayers.front() == "f0");
            else
                return false;
        },
        {"MergerData"})};

    // Gate using matrix
    auto* sm {new ActPhysics::SilMatrix {"veto"}};
    sm->Read("./veto_matrix.root");

    // auto veto {df};
    auto veto {df.Filter([&](const ActRoot::MergerData& d)
                         { return sm->IsInside(d.fSilNs.front(), d.fSP.Y(), d.fSP.Z()); },
                         {"MergerData"})};

    // Gate using cut
    ActRoot::CutsManager<int> cut;
    cut.ReadCut(0, "./Cuts/debug_hes.root");

    auto gated {veto.Filter([&](const ActRoot::MergerData& d) { return cut.IsInside(0, d.fSilEs.front(), d.fQave); },
                            {"MergerData"})};

    // And book histograms!
    // Book histograms
    auto hPID {veto.Define("ESil0", "fSilEs.front()").Histo2D(HistConfig::PID, "ESil0", "fQave")};
    auto hSP {veto.Histo2D(HistConfig::SP, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // plot
    auto* c10 {new TCanvas("c10", "Pipe1 canvas 0")};
    c10->DivideSquare(2);
    c10->cd(1);
    hPID->DrawClone("colz");
    cut.DrawAll();
    c10->cd(2);
    hSP->DrawClone("colz");
    sm->SetSyle();
    sm->Draw(true);
}
