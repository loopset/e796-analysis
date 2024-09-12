#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1.h"

#include "Math/Integrator.h"
void voigt()
{
    // Create histogram
    auto* h1 {new TH1D {"h1", "Voigt func example;X", 300, -2, 2}};
    auto* f {new TF1 {"fv", "[0] * TMath::Voigt(x - [1], [2], [3])", -2, 2}};
    f->SetParameters(100, 0, 0.1, 0);
    h1->FillRandom("fv", 100000);
    // Fit
    h1->Fit(f, "0M+");
    // Compute integral
    // auto integral {f->Integral(h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax())};
    ROOT::Math::IntegratorOneDimOptions::SetDefaultIntegrator("Gauss");
    auto integral {f->Integral(-1, 1) / h1->GetBinWidth(0)};
    std::cout << "Integral : " << integral << '\n';

    auto* c0 {new TCanvas};
    h1->Draw();
    h1->GetFunction("fv")->Draw("same");

    auto* g {new TGraphErrors};
    auto* gg {new TGraphErrors};

    // create file
    auto* fout {new TFile {"./graphs.root", "reacreate"}};
    g->Write("g0");
    gg->Write("g1");

    fout->Close();
}
