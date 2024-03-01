#include "ActKinematics.h"
#include "ActParticle.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TString.h"

TMultiGraph* BuildMultiGraph(TGraph* l, TGraph* h, const TString& title)
{
    auto* mg {new TMultiGraph};
    mg->SetTitle(title + ";#theta_{Lab} [#circ];E_{Lab} [MeV]");
    l->SetTitle("light");
    h->SetTitle("heavy");
    mg->Add(l, "l");
    mg->Add(h, "l");
    return mg;
}

void TheoKin()
{
    // A simply plotter of theoretical kinematics
    ActPhysics::Particle beam {"20O"};
    double T1 {35}; // MeV / u

    // 20O(d,t)
    ActPhysics::Kinematics dt {"20O", "2H", "3H", T1 * beam.GetAMU()};
    // dt.Print();
    auto* mdt {BuildMultiGraph(dt.GetKinematicLine3(), dt.GetKinematicLine4(), "^{20}O(d, t)")};

    // 20O(p,d)
    ActPhysics::Kinematics pd {"20O", "1H", "2H", T1 * beam.GetAMU()};
    auto* mpd {BuildMultiGraph(pd.GetKinematicLine3(), pd.GetKinematicLine4(), "^{20}O(p, d)")};

    // Plot
    auto* c0 {new TCanvas {"c0", "E796 theoretical kinematics"}};
    c0->DivideSquare(2);
    c0->cd(1);
    mdt->Draw("al");
    c0->cd(1)->BuildLegend();
    c0->cd(2);
    mpd->Draw("al");
    c0->cd(2)->BuildLegend();
}
