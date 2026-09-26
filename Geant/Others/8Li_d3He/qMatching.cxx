#include "ActKinematics.h"

#include "TCanvas.h"
#include "TGraph.h"

void qMatching()
{
    auto* k {new ActPhysics::Kinematics {"8Li(d,3He)7He@300|17"}};
    std::cout << "Qvalue : " << k->GetQValue() << '\n';
    auto* g {k->EvalQMatching(130, 700, 10)};

    auto* c0 {new TCanvas {"c0", "Q matching"}};
    g->Draw("apl");
}
