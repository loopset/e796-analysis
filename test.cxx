#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TChain.h"
#include "TF1.h"
#include "TF1Convolution.h"
#include "TMath.h"
#include "TStyle.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

double func(double* x, double* p)
{
    if(x[0] < 2)
        return 0;
    else
        return TMath::BreitWigner(x[0], 4, 0.5);
}

void test()
{
    // Gauss
    auto* fg {new TF1 {"g", "TMath::Gaus(x, 0, 0.1)", -5, 10}};
    // Lorentz
    // auto* fl {new TF1 {"l", "TMath::BreitWigner(x, 4, 0.5)", -5, 10}};
    auto* fl {new TF1 {"l", func, -5, 10, 0}};

    // Convolution
    auto* conv {new TF1Convolution {fl, fg}};
    conv->SetNofPointsFFT(1000);
    auto* fwrap {new TF1 {"fwrap", *conv, -5, 10, conv->GetNpar()}};
    fwrap->SetNpx(1000);

    // Draw
    auto* c0 {new TCanvas {"c0", "Conv canvas"}};
    fl->SetLineColor(8);
    fl->Draw();
    fg->SetLineColor(kRed);
    fg->Draw("same");
    fwrap->SetLineColor(kBlue);
    fwrap->Draw("same");
}
