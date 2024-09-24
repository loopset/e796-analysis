#include "ActKinematics.h"
#include "ActParticle.h"

#include "Rtypes.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TVirtualPad.h"

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

    // Compare also other kinematics
    ActPhysics::Kinematics pt {"20O", "p", "t", T1 * beam.GetAMU()};
    auto* mpt {new TMultiGraph};
    mpt->SetTitle(";#theta_{Lab} [#circ];E_{Lab} [MeV]");
    auto* gpt {pt.GetKinematicLine3()};
    gpt->SetTitle("^{20}O(p,t) g.s");
    auto* gdt {dt.GetKinematicLine3()};
    gdt->SetTitle("^{20}O(d,t) g.s");
    dt.SetEx(18);
    auto* gdtex {dt.GetKinematicLine3()};
    gdtex->SetTitle("^{20}O(d,t) E_{x} = 18 MeV");
    auto* leg {new TLegend {0.3, 0.3}};
    // Add
    for(auto* g : {gpt, gdt, gdtex})
    {
        mpt->Add(g);
        leg->AddEntry(g);
    }

    // Plot
    auto* c0 {new TCanvas {"c0", "E796 theoretical kinematics"}};
    c0->DivideSquare(4);
    c0->cd(1);
    mdt->Draw("al");
    c0->cd(1)->BuildLegend();
    c0->cd(2);
    mpd->Draw("al");
    c0->cd(2)->BuildLegend();
    c0->cd(3);
    mpt->SetMinimum(0);
    mpt->SetMaximum(60);
    mpt->Draw("al plc pmc");
    gPad->Update();
    // Draw line of cut for 20O(d,t) @ 12 MeV approx
    auto* line {new TLine {gPad->GetUxmin(), 12, gPad->GetUxmax(), 12}};
    line->SetLineWidth(2);
    line->SetLineStyle(2);
    line->SetLineColor(kRed);
    line->Draw();
    leg->AddEntry(line, "PID cut", "l");
    leg->Draw();
}
