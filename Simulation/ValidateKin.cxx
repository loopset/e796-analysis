#include "ActKinematics.h"

#include "TCanvas.h"

void ValidateKin()
{
    // Direct kinematics
    ActPhysics::Kinematics d {"d", "11Li", "p", 700};
    // Get angles
    auto* gd {d.GetThetaLabvsThetaCMLine()};

    // Inverse kinematics
    ActPhysics::Kinematics i {"11Li", "d", "p", 700};
    auto* gi {i.GetThetaLabvsThetaCMLine()};

    // Plot
    auto* c0 {new TCanvas {"c0", "Check kinematics"}};
    c0->DivideSquare(4);
    c0->cd(1);
    gd->Draw("al");
    c0->cd(2);
    gi->Draw("al");
}
