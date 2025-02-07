#include "ActSRIM.h"

#include "TGraphErrors.h"
#include "TMath.h"

void debug()
{
    // Read 2h in sil table
    ActPhysics::SRIM srim;
    srim.ReadTable("si", "../../Calibrations/SRIMData/raw/2H_silicon.txt");

    auto* g {new TGraphErrors};
    g->SetTitle("Eloss of 2H in Sil;Eini;#DeltaE");

    // Thickness
    double si {500e-3};
    double theta {40 * TMath::DegToRad()};

    double ini {1};
    double end {20};
    double step {0.1};
    for(double it = ini; it <= end; it += step)
    {
        auto out {srim.Slow("si", it, si, theta)};
        g->AddPoint(it, it - out);
    }

    g->Draw("apl");
}
