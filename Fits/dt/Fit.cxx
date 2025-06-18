#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"
#include "Rtypes.h"

#include "TROOT.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "FitInterface.h"
#include "FitModel.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <string>
#include <vector>

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
    ROOT::RDataFrame pd {"Sel_Tree", "./Inputs/Cont/tree_20O_d_d_as_d_t.root"};
    auto hpd {pd.Histo1D(E796Fit::Exdt, "Ex")};
    // ROOT::RDataFrame ddphase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -4)};
    // auto hPSdd {ddphase.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    // Fitters::TreatPS(hEx.GetPtr(), hPSdd.GetPtr());
    // // Contamination from 20O(p,d)
    // std::vector<double> conts {0, 1.4, 3.2, 4.5};
    // std::vector<ROOT::RDF::RResultPtr<TH1D>> hconts;
    // for(const auto& cont : conts)
    // {
    //     auto file {gSelector->GetApproxSimuFile("20O", "1H", "2H", cont, -3)};
    //     ROOT::RDataFrame dfc {"SimulationTTree", file};
    //     auto hCont {dfc.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    //     hCont->SetNameTitle("hCont", TString::Format("20O(p,d) %.2f cont", cont));
    //     Fitters::TreatPS(hEx.GetPtr(), hCont.GetPtr(), 0);
    //     hconts.push_back(hCont);
    // }

    // Sigma interpolators
    Interpolators::Sigmas sigmas {gSelector->GetSigmasFile("2H", "3H").Data()};

    // Interface for fitting!
    Fitters::Interface inter;
    double sigma {0.364}; // common guess for all states
    double offset {0.0};  // != 0 only when using ExLegacy
    inter.AddState("g0", {400, 0 - offset, sigma}, "5/2+");
    inter.AddState("g1", {10, 1.4 - offset, sigma}, "1/2+");
    inter.AddState("g2", {110, 3.2 - offset, sigma}, "(1/2,3/2)-");
    inter.AddState("v0", {60, 4.5 - offset, sigma, 0.1}, "3/2-");
    inter.AddState("v1", {60, 6.7 - offset, sigma, 0.845}, "?");
    inter.AddState("v2", {60, 7.9 - offset, sigma, 0.1}, "?");
    inter.AddState("v3", {60, 8.9 - offset, sigma, 0.1}, "?");
    inter.AddState("v4", {60, 11 - offset, sigma, 0.1}, "?");
    inter.AddState("v5", {40, 12.2 - offset, sigma, 0.1}, "?");
    inter.AddState("v6", {40, 13.8 - offset, sigma, 0.1}, "?");
    inter.AddState("v7", {20, 14.9 - offset, sigma, 0.1}, "?");
    // inter.AddState("v8", {20, 16. - offset, sigma, 0.}, "Cont0");
    // inter.AddState("v9", {20, 17. - offset, sigma, 0.}, "Cont1");
    // inter.AddState("v10", {20, 20.1 - offset, sigma, 0.}, "Cont2");
    // inter.AddState("v11", {20, 22.6 - offset, sigma, 0.}, "Cont3");
    inter.AddState("ps0", {1.5});
    inter.AddState("ps1", {0.1});
    inter.AddState("ps2", {0.1});
    // for(int i = 0; i < conts.size(); i++)
    //     inter.AddState(TString::Format("ps%d", i + 2).Data(), {0.1});
    inter.EndAddingStates();
    // Wider mean margin
    inter.SetOffsetMeanBounds(0.5);
    inter.ReadPreviousFit("./Outputs/fit_" + gSelector->GetFlag() + ".root");
    // Eval correct sigma
    inter.EvalSigma(sigmas.GetGraph());
    // Fix Gamma for v1
    // inter.SetBounds("v1", 3, {0, 0.2});
    // inter.SetFix("v1", 3, true); // if not fixed, gets larger and disturbs v2
    // this value has been obtained from the larger PID fit
    // inter.SetBounds("v2", 0, {40, 100});
    // inter.SetFix("ps0", 0, true);
    inter.SetFixAll(2, true);
    inter.SetBounds("v2", 3, {0, 0.1});
    inter.SetBounds("v3", 3, {0, 0.1});
    inter.SetBounds("v5", 3, {0, 0.25});
    inter.SetBounds("v6", 3, {0, 0.1});
    inter.SetBounds("v7", 3, {0, 0.1});
    // for(const auto& s : {"v8", "v9", "v10", "v11"})
    // {
    //     // inter.SetFix(s, 1, true);
    //     inter.SetFix(s, 3, true);
    // }
    inter.Write("./Outputs/interface.root");

    // Fitting range
    double exmin {-5};
    double exmax {25};
    // Model
    std::vector<TH1D> hPSs {*hPS, *hPS2, *hpd};
    // for(auto hc : hconts)
    //     hPSs.push_back(*hc);
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), hPSs};
    model.SetUseSpline(true);
    // Run!
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    ("./Outputs/fit_" + gSelector->GetFlag() + ".root"), "20O(d,t)", {}, false);

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
