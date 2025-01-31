#include "TCanvas.h"
#include "TF2.h"
#include "TProfile2D.h"

void test()
{
    auto* func {new TF2 {"func", "120 + (y - 120) * TMath::Tan(3 * TMath::DegToRad())", 0, 300, 0, 300}};
    auto* h {new TProfile2D {"h", "X value per (Y,Z) pair;Y;Z", 200, 0, 300, 200, 0, 300}};
    for(double y = 0; y < 300; y += 1)
    {
        for(double z = 0; z < 300; z += 1)
            h->Fill(y, z, func->Eval(y, z));
    }

    auto* c0 {new TCanvas {"c0", "test canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    func->Draw();
    c0->cd(2);
    h->Draw("colz");
}
