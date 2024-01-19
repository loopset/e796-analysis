#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

void BasicRDataFrame()
{
    // Enable multithreading !
    // (always before constructing any RDF)
    ROOT::EnableImplicitMT();
    // Construct it: TTree name + path to file
    ROOT::RDataFrame df {"SimulationTTree", "./Outputs/transfer_TRIUMF_Eex_0.000_nPS_0_pPS_0.root"};
    // Print columns names
    df.Describe().Print();

    // Always BOOK actions before requiring them
    // Action = histogram, filter, etc...
    // Node = a picture of the original RDF after applying an action
    auto node1 {df.Filter("theta3CM < 180")};
    // Histogram: ({histogram model}, "column X", "columnY")
    auto hEx {node1.Histo1D({"hEx", "Ex histogram;E_{x} [MeV]", 150, -5, 10}, "Eex")};

    // And finally draw
    auto* c1 {new TCanvas("c1", "Excitation energy canvas")};
    hEx->DrawClone();
    // RDF returns SMART POINTERS to objects: they get automatically deleted after macro ends
    // workaraound: draw a _traditional_ pointer with the DrawClone() method
}
