#include "ActJoinData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"

void PID()
{
    ROOT::EnableImplicitMT();

    ActRoot::JoinData data {"./../../configs/merger.runs"};
    ROOT::RDataFrame d {*data.Get()};

    // Gate on events stopped in first layer
    auto df {d.Filter("fSilLayers.size() == 1")};
    // df.Describe().Print();
    df = df.Define("ESi0", "fSilEs.front()");

    // Book histograms
    auto dleft {df.Filter("fSilLayers.front() == \"l0\"")};
    auto hLeft {dleft.Histo2D(HistConfig::ChangeTitle(HistConfig::PID, "Left PID"), "ESi0", "fQave")};
    auto dfront {df.Filter("fSilLayers.front() == \"f0\"")};
    auto hFront {dfront.Histo2D(HistConfig::ChangeTitle(HistConfig::PID, "Front PID"), "ESi0", "fQave")};

    // plotting
    auto* c1 {new TCanvas("c1", "PID canvas")};
    c1->DivideSquare(2);
    c1->cd(1);
    hLeft->DrawClone("colz");
    c1->cd(2);
    hFront->DrawClone("colz");
}
