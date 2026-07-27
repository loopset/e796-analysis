#include "ActKinematics.h"

#include "TCanvas.h"
#include "TGraph.h"

#include <iostream>

void qmatching()
{
    auto* k {new ActPhysics::Kinematics {"16N(d,3He)@300"}};
    std::cout << "Qvalue : " << k->GetQValue() << '\n';
    auto* gmatch {k->EvalQMatching(400, 700, 30)};

    auto* c0 {new TCanvas {"c0", "qmatching"}};
    gmatch->Draw("a*l");
}