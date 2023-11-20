#include "ActJoinData.h"
#include "ROOT/RDataFrame.hxx"
#include "TCanvas.h"
#include "TROOT.h"
void PID()
{
    ROOT::EnableImplicitMT();

    ActRoot::JoinData data {"../configs/merger.runs"};
    ROOT::RDataFrame d {*data.Get()};
 
    // Gate on events stopped in first layer
    auto df {d.Filter("fSilLayers.size() == 1")};
    // df.Describe().Print();
    df = df.Define("ESi0", "fSilEs.front()");
    
    // Book histograms
    auto dleft {df.Filter("fSilLayers.front() == \"l0\"")};
    auto hLeft {dleft.Histo2D({"hLeft", "Left PID;E_{Left} [MeV];Q_{ave} [au / mm]", 200, 0, 30, 300, 0, 1000},
                              "ESi0", "fQave")};
    auto dfront {df.Filter("fSilLayers.front() == \"f0\"")};
    auto hFront {dfront.Histo2D({"hFront", "Front PID;E_{Si0} [MeV];Q_ave [au / mm]", 200, 0, 30, 300, 0, 2000},
                                "ESi0", "fQave")};

    // plotting
    auto* c1 {new TCanvas("c1", "PID canvas")};
    c1->DivideSquare(2);
    c1->cd(1);
    hLeft->DrawClone("colz");
    c1->cd(2);
    hFront->DrawClone("colz");
}
