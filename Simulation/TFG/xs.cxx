#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "ActCrossSection.h"
#include "TH1.h"
void xs()
{
    auto* gs {new TGraphErrors {"./Inputs/xs/s12_p1i.dat", "%lg %lg"}};
    gs->SetTitle("s_{1/2}");
    auto* gp {new TGraphErrors {"./Inputs/xs/p12_p1i.dat", "%lg %lg"}};
    gp->SetTitle("p_{1/2}");

    auto* mg {new TMultiGraph};
    mg->SetTitle(";#theta_{CM} [#circ];d#sigma/d#Omega [mb/sr]");
    for(auto* g : {gs, gp})
    {
        g->SetLineWidth(2);
        mg->Add(g);
    }

    // Sampling
    ActSim::CrossSection xs {};
    xs.ReadFile("./Inputs/xs/p12_p1i.dat");
    xs.Draw();

    auto* hThetaCM {new TH1D {"hThetaCM", "CM;#theta_{CM};Counts", 300, 0, 180}};
    for(int i = 0; i < 1000000; i++)
    {
        hThetaCM->Fill(xs.SampleHist());
    }


    auto* c0 {new TCanvas {"c0", "xs canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    mg->Draw("apl plc pmc");
    c0->cd(2);
    hThetaCM->Draw();
}
