#include "TAxis.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TRandom.h"
#include "TString.h"

#include <vector>

void LateralDiffusion()
{
    std::vector<double> zetas {1, 5, 10, 15, 20, 25, 30};
    double D {0.5};
    double vdrift {10};
    std::vector<double> sigmas;
    for(const auto& zeta : zetas)
    {
        sigmas.push_back(TMath::Sqrt(2 * D * zeta / vdrift));
        std::cout << "Sigma : " << sigmas.back() << '\n';
    }

    // Create graphs
    auto* gint {new TGraphErrors};
    gint->SetTitle("Integral;Z;Integral");
    // Set threshold
    double thresh {0.15};
    auto* gabove {new TGraphErrors};
    gabove->SetTitle("Integral above limit;Z;Integral");
    // Create functions
    std::vector<TF1*> funcs;
    int idx {};
    for(const auto& sigma : sigmas)
    {
        const auto& z {zetas[idx]};
        double lim {100};
        auto* f {new TF1 {"f", TString::Format("TMath::Gaus(x, 0, %.4f, true)", sigma), -lim, lim}};
        funcs.push_back(f);
        // Integral
        auto integral {f->Integral(f->GetXmin(), f->GetXmax())};
        gint->AddPoint(z, integral);
        // Above point
        auto ylow {f->GetX(thresh, -lim, 0)};
        auto yup {f->GetX(thresh, 0, lim)};
        auto intInterval {f->Integral(ylow, yup)};
        gabove->AddPoint(z, intInterval);
        idx++;
    }

    // Set styles
    for(auto* g : {gint, gabove})
    {
        g->SetMarkerStyle(24);
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Lateral diffusion canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    gint->Draw("apl");
    c0->cd(2);
    idx = 0;
    for(auto& f : funcs)
    {
        f->SetNpx(1000);
        if(idx == 0)
        {
            f->Draw();
            f->GetXaxis()->SetRangeUser(-5, 5);
        }
        else
            f->Draw("same");
        idx++;
    }
    c0->cd(3);
    gabove->Draw("apl");
}
