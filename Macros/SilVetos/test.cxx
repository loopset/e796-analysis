#include "TCanvas.h"
#include "TFile.h"

#include "./GetContourFuncs.cxx"

void test()
{
    auto* file {new TFile {"./Inputs/antiveto_histograms.root"}};
    auto* h {file->Get<TH1D>("pz5")};
    
    FindBestFit(h,5, 0.1, "expo");
    // Draw
    auto* c0 {new TCanvas {"c0", "test"}};
    h->Draw();
}
