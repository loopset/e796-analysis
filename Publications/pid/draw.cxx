#include "TCanvas.h"
#include "TFile.h"
#include "TH2.h"
void draw()
{
    // Read the histogram
    auto* f {new TFile {"./Inputs/pid_front.root"}};
    auto* h {f->Get<TH2D>("hPID")};

    // Format
    h->SetTitle("");
    h->GetYaxis()->SetTitle("#bar{Q} [mm^{-1}]");
    h->SetStats(false);

    // Draw 
    auto* c0 {new TCanvas {"c0", "PID canvas"}};
    c0->SetWindowSize(700, 600);
    h->Draw("colz");

    c0->SaveAs("./Outputs/pid_front.pdf");
    c0->SaveAs("./Outputs/pid_front.eps");
}
