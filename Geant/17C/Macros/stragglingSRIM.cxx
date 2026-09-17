#include "ActSRIM.h"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TMath.h"

void stragglingSRIM()
{
    ActPhysics::SRIM srim {};
    srim.ReadSRIM("p", "../Inputs/protons_srim.dat");

    double Eini {5};
    double Eend {15};
    double Estep {2};
    double dist {300}; // mm
    int Nit {10000};

    auto* gstragg {new TGraphErrors};
    gstragg->SetTitle("Straggling;E [MeV];#sigma [MeV]");
    for(double E = Eini; E <= Eend; E += Estep)
    {
        auto* h {new TH1D {"h", "it", 200, -5, 5}};
        for(int i = 0; i < Nit; i++)
        {
            auto eafter {srim.SlowWithStraggling("p", E, dist)};
            auto eslow {srim.Slow("p", E, dist)};
            auto diff {eafter - eslow};
            h->Fill(diff);
        }
        gstragg->AddPoint(E, h->GetStdDev());
        delete h;
    }

    auto* c0 {new TCanvas {"c0", "Straggling"}};
    gstragg->Draw("apl");
}