#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TSpline.h"

// The variable spline has to exist during all
// execution time because once plotted, TCanvas requires it to be available
TSpline3* spline {};

void SampleTheoXS()
{
    // 1-> Read TGraphErrors
    auto* g {new TGraphErrors("./Inputs/TheoXS/5.5MeV/angp32nospin.dat", "%lg %lg")};
    // 2-> Convert to TSpline: interpolation class
    spline = new TSpline3("spline", g, "b2,e2", 0, 0);
    // 3-> Get TF1: use spline as input for a general TF in 1 dim
    // Using a LAMBDA function
    auto* func {new TF1(
        "func", [&](double* x, double* p) { return spline->Eval(x[0]); }, 0, 180, 0)};
    func->SetTitle("Diff xs;#theta_{CM} [#circ];Diff xs");
    func->SetLineColor(kPink);
    func->SetLineWidth(2);

    // Get random from TF1!!
    auto* hist {new TH1F("hist", "Sampled histogram;#theta_{CM} [deg]", 150, 0, 180)};
    for(int i = 0; i < 10000; i++)
    {
        auto sampled {func->GetRandom()};
        hist->Fill(sampled);
    }

    // plotting
    auto* c1 {new TCanvas("c1", "Canvas")};
    c1->DivideSquare(2);
    c1->cd(1);
    func->Draw();
    c1->cd(2);
    hist->Draw();
}
