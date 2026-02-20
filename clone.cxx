#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"

void clone()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", "./PostAnalysis/RootFiles/Pipe3/tree_20O_2H_3H_front_juan_RPx.root"};

    auto h {df.Histo1D("Ex")};

    
    auto* c0 {new TCanvas {"c0", "test clone"}};
    c0->DivideSquare(4);
    c0->cd(1);
    h->DrawClone();
}
