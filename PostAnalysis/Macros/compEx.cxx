#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"
#include "TCanvas.h"
#include "../../Selector/Selector.h"

#include "../HistConfig.h"
void compEx() 
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame dfo {"Final_Tree", "/media/Data/E796v2/PostAnalysis/RootFiles/Old/Pipe2/tree_beam_20O_target_2H_light_3H_front.root"};
    ROOT::RDataFrame dfh {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")};

    auto hExo {dfo.Histo1D(HistConfig::Ex, "Ex")};
    auto hExh {dfh.Histo1D(HistConfig::Ex, "Ex")};

    auto* c0 {new TCanvas {"c0", "Compare Ex"}};
    hExo->DrawClone();
    hExh->SetLineColor(kRed);
    hExh->DrawClone("same");
}
