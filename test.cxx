#include "ActSRIM.h"

#include "TF1.h"
#include "TH1.h"
#include "TRandom.h"
#include "TStyle.h"
void test()
{
    ActPhysics::SRIM srim;
    // srim.SetUseSpline();
    srim.ReadTable("test", "./Calibrations/SRIMData/raw/1H_952mb_mixture.txt");

    auto* hOff {new TH1D {"hOff", "Off", 2200, 0, 180}};
    auto* hOn {(TH1D*)hOff->Clone("hOn")};

    double Tini {6};
    double thick {256}; // mm
    gRandom->SetSeed();
    for(int i = 0; i < 100000; i++)
    {
        hOff->Fill(srim.Slow("test", Tini, thick));
        hOn->Fill(srim.SlowWithStraggling("test", Tini, thick));
    }

    gStyle->SetOptFit();
    // hOff->DrawNormalized();
    hOn->SetLineColor(8);
    hOn->Fit("gaus");
    hOn->GetFunction("gaus")->SetNpx(1000);
}
