#include "ActKinematics.h"
#include "ActParticle.h"

#include "TGraph.h"
#include "TLegend.h"
#include "TMultiGraph.h"

#include <vector>

void kin()
{
    ActPhysics::Particle pb {"11Li"};
    double Tbeam {7.5 * pb.GetAMU()};
    // Gs
    ActPhysics::Kinematics k0 {"11Li", "d", "p", Tbeam};
    // 1st
    ActPhysics::Kinematics k1 {"11Li", "d", "p", Tbeam, 0.130};
    // 2nd
    ActPhysics::Kinematics k2 {"11Li", "d", "p", Tbeam, 0.435};

    std::vector<std::string> labels {"g.s", "0.130", "0.435"};
    auto* leg {new TLegend {0.3, 0.3}};
    leg->SetHeader("E_{x} [MeV]", "C");
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    std::vector<int> ls {1, 2, 7};
    std::vector<int> colors {46, 8, 9};
    auto* mg {new TMultiGraph};
    mg->SetTitle(";#theta_{Lab} [deg];T_{Lab} [MeV]");
    int idx {};
    for(auto& k : {&k0, &k1, &k2})
    {
        auto* g {k->GetKinematicLine3()};
        g->SetLineWidth(3);
        g->SetLineStyle(ls[idx]);
        g->SetLineColor(colors[idx]);
        leg->AddEntry(g, labels[idx].c_str(), "l");
        mg->Add(g);
        idx++;
    }

    mg->Draw("al");
    leg->Draw();
}
