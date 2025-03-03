#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TROOT.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "FitInterface.h"
#include "FitModel.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <string>

#include "/media/Data/E796v2/Fits/FitHist.h"
#include "/media/Data/E796v2/Selector/Selector.h"

void Fit()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")};
    auto hEx {df.Histo1D(E796Fit::Exdt, "Ex")};
    // Phase spaces
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 1, 0)};
    auto hPS {phase.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    hPS->SetNameTitle("hPS", "1n PS");
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr());
    ROOT::RDataFrame phase2 {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 2, 0)};
    auto hPS2 {phase2.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    hPS2->SetNameTitle("hPS2", "2n PS");
    Fitters::TreatPS(hEx.GetPtr(), hPS2.GetPtr());
    // Contamination of 20O(p,d) gs
    ROOT::RDataFrame cont {"SimulationTTree", gSelector->GetSimuFile("20O", "1H", "2H", 0, -3, 0)};
    auto hCont {cont.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    hCont->SetNameTitle("hCont", "20O(p,d) gs cont");
    Fitters::TreatPS(hEx.GetPtr(), hCont.GetPtr(), 0);

    // Sigma interpolators
    Interpolators::Sigmas sigmas {gSelector->GetSigmasFile("2H", "3H").Data()};

    // Interface for fitting!
    Fitters::Interface inter;
    double sigma {0.364}; // common guess for all states
    double offset {0.0}; // != 0 only when using ExLegacy
    inter.AddState("g0", {400, 0 - offset, sigma}, "5/2+");
    inter.AddState("g1", {10, 1.4 - offset, sigma}, "1/2+");
    inter.AddState("g2", {110, 3.2 - offset, sigma}, "(1/2,3/2)-");
    inter.AddState("v0", {60, 4.5 - offset, sigma, 0.1}, "3/2-");
    inter.AddState("v1", {60, 6.7 - offset, sigma, 0.1}, "?");
    inter.AddState("v2", {60, 7.9 - offset, sigma, 0.1}, "?");
    inter.AddState("v3", {60, 8.9 - offset, sigma, 0.1}, "?");
    inter.AddState("v4", {60, 11 - offset, sigma, 0.1}, "?");
    // inter.AddState("v5", {5, 12.8, sigma, 0.1}, "?");
    inter.AddState("v5", {20, 14.9 - offset, sigma, 0.1}, "?");
    // inter.AddState("v6", {10, 16 - offset, sigma, 0.1}, "?");
    // inter.AddState("v8", {10, 17.5, sigma, 0.1}, "cont");
    inter.AddState("ps0", {0.1});
    inter.AddState("ps1", {0.1});
    inter.AddState("ps2", {0.1});
    inter.EndAddingStates();
    // Wider mean margin
    inter.SetOffsetMeanBounds(0.5);
    inter.ReadPreviousFit("./Outputs/fit_" + gSelector->GetFlag() + ".root");
    // Eval correct sigma
    inter.EvalSigma(sigmas.GetGraph());
    // Fix all sigmas (3rd parameter, 2nd index of vector)
    inter.SetFixAll(2, true);
    inter.SetBounds("v2", 3, {0, 0.1});
    inter.SetBounds("v3", 3, {0, 0.1});
    inter.Write("./Outputs/interface.root");

    // Fitting range
    double exmin {-5};
    double exmax {25};
    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS, *hPS2, *hCont}};
    // Run!
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    ("./Outputs/fit_" + gSelector->GetFlag() + ".root"), "20O(d,t)");

    gPad->GetListOfPrimitives()->FindObject("TPave")->Delete();
    // // Draw Sn and S2n lines
    // gPad->Update();
    // gPad->cd();
    // auto* line {new TLine {3.956, gPad->GetUymin(), 3.956, gPad->GetUymax()}};
    // line->SetLineWidth(2);
    // line->SetLineColor(kOrange);
    // line->Draw("same");
    // // auto* line2 {new TLine {17.069, gPad->GetUymin(), 17.069, gPad->GetUymax()}};
    // // line2->SetLineWidth(2);
    // // line2->SetLineColor(kOrange);
    // // line2->Draw("same");
    // auto* line3 {new TLine {12, gPad->GetUymin(), 12, gPad->GetUymax()}};
    // line3->SetLineWidth(2);
    // line3->SetLineColor(kCyan);
    // line3->Draw("same");

    gSelector->SendToWebsite("dt.root", gPad, "cFit");
}
