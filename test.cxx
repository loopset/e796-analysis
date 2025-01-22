#include "ActSRIM.h"

#include "TF1.h"
#include "TH1.h"
#include "TRandom.h"
#include "TStyle.h"
void test()
{
    ActPhysics::SRIM srim;
    // srim.SetUseSpline();
    srim.ReadTable("20Mg", "./Calibrations/SRIMData/raw/20Mg_butante_266mbar.txt");

    auto* hOff {new TH1D {"hOff", "Off", 800, 80, 180}};
    auto* hOn {(TH1D*)hOff->Clone("hOn")};

    double Tini {140};
    double thick {0.1}; // mm
    gRandom->SetSeed();
    for(int i = 0; i < 1000000; i++)
    {
        hOff->Fill(srim.Slow("20Mg", Tini, thick));
        hOn->Fill(srim.SlowWithStraggling("20Mg", Tini, thick));
    }

    gStyle->SetOptFit();
    // hOff->DrawNormalized();
    hOn->SetLineColor(8);
    hOn->Fit("gaus");
    hOn->GetFunction("gaus")->SetNpx(1000);
}
