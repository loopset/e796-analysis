#include "ActSilMatrix.h"

#include "TAttLine.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TLegend.h"

void CompareSilMatrix()
{
    ActPhysics::SilMatrix sma; // using all data
    sma.Read("./silmatrix.root");
    sma.SetName("All data");
    sma.SetSyle(true, kSolid, 2, 0);
    sma.Print();

    ActPhysics::SilMatrix sm0; // using data RP.X() < 256. / 3
    sm0.Read("./silmatrix_part0.root");
    sm0.SetName("First third");
    sm0.SetSyle(false, kDashed, 4, 0);
    sm0.Print();

    ActPhysics::SilMatrix sm1; // using data RP.X() > 256. * 2. / 3
    sm1.Read("./silmatrix_part1.root");
    sm1.SetName("Last third");
    sm1.SetSyle(false, kDotted, 4, 0);
    sm1.Print();

    // build legend
    auto* lall {new TLine()};
    lall->SetLineWidth(2);
    lall->SetLineStyle(kSolid);

    auto* l0 {new TLine()};
    l0->SetLineWidth(4);
    l0->SetLineStyle(kDashed);
    
    auto* l1 {new TLine()};
    l1->SetLineWidth(4);
    l1->SetLineStyle(kDotted);

    auto* leg {new TLegend(0.3, 0.3)};
    leg->AddEntry(lall, "All", "l");
    leg->AddEntry(l0, "RP.X < 85 mm", "l");
    leg->AddEntry(l1, "RP.X > 171 mm", "l");

    // plot
    auto* cm {new TCanvas("cm", "Sil matrix canvas")};
    sma.Draw();
    sm0.Draw(true);
    sm1.Draw(true);
    // leg->Draw("same");
}
