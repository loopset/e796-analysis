#include "ActKinematics.h"

#include "Rtypes.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TMath.h"
void test()
{
    ActPhysics::Kinematics k {"20O", "d", "t", 700};
    auto* gtheo {k.GetKinematicLine3()};


    // Compute points
    double emin {50};
    double emax {150};
    double step {0.5};
    auto* g {new TGraph};
    for(double e = emin; e <= emax; e += step)
    {
        auto theta {k.ComputeTheta3FromT3(e)};
        g->AddPoint(theta * TMath::RadToDeg(), e);
    }
    g->SetLineColor(kBlue);
    g->SetLineWidth(4);
    
    // Draw
    auto* c0 {new TCanvas {"c0", "Kinematic canvas"}};
    gtheo->Draw("al");
    g->Draw("l");
}
