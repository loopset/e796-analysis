#include "ActSRIM.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TString.h"
#include "TStyle.h"

#include <vector>

void test()
{
    TH1::AddDirectory(false);

    auto* srim {new ActPhysics::SRIM};
    srim->SetUseSpline();
    srim->ReadTable("p", "../Calibrations/SRIMData/raw/1H_952mb_mixture.txt");
    srim->Draw();

    auto dist {250}; // mm
    std::vector<TH1D*> hs;
    auto* gs {new TGraphErrors};
    gs->SetTitle("Straggling graph;E;Sigma");
    for(double e = 3; e <= 20; e += 0.1)
    {
        // Histogram
        auto* h {new TH1D {"h", TString::Format("T_{ini} = %.2f;T_{after} [MeV]", e), 600, 0, 30}};
        // Fill
        for(int i = 0; i < 100000; i++)
        {
            auto after {srim->SlowWithStraggling("p", e, dist)};
            h->Fill(after);
        }
        // Fit
        h->Fit("gaus", "0QM+");
        auto* f {h->GetFunction("gaus")};
        f->ResetBit(TF1::kNotDraw);
        f->SetNpx(1000);
        auto mean {f->GetParameter("Mean")};
        auto sigma {f->GetParameter("Sigma")};
        gs->AddPoint(e, sigma);

        hs.push_back(h);
    }

    // Draw
    gStyle->SetOptFit(1);
    auto* c0 {new TCanvas {"c0", "Straggling canvas"}};
    // c0->DivideSquare(hs.size());
    // for(int i = 0; i < hs.size(); i++)
    // {
    //     c0->cd(i + 1);
    //     hs[i]->Draw();
    // }

    auto* c1 {new TCanvas {"c1", "Straggling canvas"}};
    gs->SetLineWidth(2);
    gs->SetMarkerStyle(24);
    gs->Draw("apl");
}
