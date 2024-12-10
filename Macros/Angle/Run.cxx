#include "ActColors.h"
#include "ActKinematics.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TLine.h"
#include "TMath.h"
#include "TProfile.h"
#include "TROOT.h"
#include "TString.h"
#include "TStyle.h"
#include "TVirtualPad.h"

#include <ios>
#include <memory>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"
#include "./FitTH2.h"

std::pair<double, double> FitGS(TH1D* h)
{
    auto bmax {h->GetMaximumBin()};
    auto gs {h->GetBinCenter(bmax)};
    double width {2.5};
    h->Fit("gaus", "0Q+", "", gs - width, gs + width);
    auto* f {h->GetFunction("gaus")};
    f->ResetBit(TF1::kNotDraw);
    f->SetLineColor(kMagenta);
    auto mean {f->GetParameter(1)};
    auto sigma {f->GetParameter(2)};
    auto factor {2.35 * 2.75};
    return {mean - (sigma * factor) / 2, mean + (sigma * factor) / 2};
}

TF1* FitCorr1(TProfile* h, double min, double max)
{
    h->Fit("pol2", "0Q+", "", min, max);
    auto* f {h->GetFunction("pol2")};
    f->ResetBit(TF1::kNotDraw);
    return f;
}

TF1* FitCorr2(TProfile* h, double min, double max)
{
    h->Fit("pol1", "0Q+", "", min, max);
    auto* f {h->GetFunction("pol1")};
    f->ResetBit(TF1::kNotDraw);
    return f;
}

void Run()
{
    gStyle->SetOptFit();
    // Read data
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Final_Tree", gSelector->GetAnaFile(2, false)};

    // Define new columns
    bool isEl {gSelector->GetIsElastic()};
    std::cout << BOLDYELLOW << "isElastic ? " << std::boolalpha << isEl << RESET << '\n';
    ActPhysics::Kinematics kin {gSelector->GetBeam(), gSelector->GetTarget(), gSelector->GetLight(), 700};
    std::vector<ActPhysics::Kinematics> vks {df.GetNSlots()};
    for(auto& k : vks)
        k = kin;
    // Define theoretical Theta3
    auto def {df.DefineSlot("ThetaTheo",
                            [&](unsigned int slot, double EBeam, double EVertex, double ExLegacy, float thetalegacy)
                            {
                                vks[slot].SetBeamEnergy(EBeam);
                                return vks[slot].ComputeTheta3FromT3(EVertex) * TMath::RadToDeg();
                            },
                            {"EBeam", "EVertex", "ExLegacy", "fThetaLegacy"})
                  .Define("Diff", "ThetaTheo - fThetaLegacy")};

    // Book histograms
    auto hEx {def.Histo1D(HistConfig::Ex, "Ex")};
    auto hExLeg {def.Histo1D(HistConfig::Ex, "ExLegacy")};
    auto hExCMLeg {def.Histo2D(HistConfig::ExThetaCM, "ThetaCMLegacy", "ExLegacy")};
    auto hExRPxLeg {def.Histo2D(HistConfig::ExRPx, "fRP.fCoordinates.fX", "ExLegacy")};
    // Fit g.s
    auto fit {FitGS(hExLeg.GetPtr())};
    auto [min, max] {fit};
    // Now with g.s selected
    auto gateGS {[=](double exlegacy) { return fit.first <= exlegacy && exlegacy <= fit.second; }};
    // Models of histograms
    ROOT::RDF::TH2DModel mDiffRPx {
        "hDiffRPx", "#delta#theta_{1} vs RP.X();RP.X() [mm];#delta#theta_{1} [#circ]", 200, -10, 300, 100, -10, 10};
    ROOT::RDF::TH2DModel mTheoLeg {
        "hTheoLeg", "Correlations;#theta_{legacy} [#circ];#theta_{theo} [#circ]", 250, 0, 90, 250, 0, 90};
    auto hDiffRPx {def.Filter(gateGS, {"ExLegacy"}).Histo2D(mDiffRPx, "fRP.fCoordinates.fX", "Diff")};
    auto hTheoLeg {def.Filter(gateGS, {"ExLegacy"}).Histo2D(mTheoLeg, "fThetaLegacy", "ThetaTheo")};
    // Profile in DiffRPx
    auto* px {hDiffRPx->ProfileX("px")};
    // auto* func1 {FitCorr1(px, 20, 210)};
    FitTH2 fit1 {hDiffRPx.GetPtr(), 3};
    auto func1 {fit1.Fit()};

    // Define new columns
    def = def.Define("ThetaLightRP", [&](float thetalegacy, float rpx) { return thetalegacy + func1->Eval(rpx); },
                     {"fThetaLegacy", "fRP.fCoordinates.fX"})
              .Define("Diff2", "ThetaTheo - ThetaLightRP");
    // New histogram for last correction
    ROOT::RDF::TH2DModel mDiffTheta {"hDiffThetaRP",
                                     "#delta#theta_{2} vs #theta_{RP};#theta_{RP} [#circ];#delta#theta_{2} [#circ]",
                                     250,
                                     0,
                                     90,
                                     200,
                                     -10,
                                     10};
    auto hTheoThetaRP {def.Filter(gateGS, {"ExLegacy"}).Histo2D(mTheoLeg, "ThetaLightRP", "ThetaTheo")};
    hTheoThetaRP->GetXaxis()->SetTitle("#theta_{RP} [#circ]");
    auto hDiffThetaRP {def.Filter(gateGS, {"ExLegacy"}).Histo2D(mDiffTheta, "ThetaLightRP", "Diff2")};
    auto gDiffThetaRP {def.Filter(gateGS, {"ExLegacy"}).Graph("ThetaLightRP", "Diff2")};
    auto hDiffThetaLeg {def.Filter(gateGS, {"ExLegacy"}).Histo2D(mDiffTheta, "fThetaLegacy", "Diff")};
    hDiffThetaLeg->GetXaxis()->SetTitle("#theta_{Leg} [#circ]");
    hDiffThetaLeg->SetTitle("#delta#theta_{1} vs #theta_{Leg}");
    hDiffThetaLeg->GetYaxis()->SetTitle("#delta#theta_{1} [#circ]");
    // Profile
    auto* px2 {hDiffThetaRP->ProfileX("px2")};
    // auto* func2 {FitCorr2(px2, 0, 60)};

    int npar {(isEl) ? 1 : 2};
    FitTH2 fitth2 {hDiffThetaRP.GetPtr(), npar};
    auto func2 {fitth2.Fit()};

    // Correct
    def = def.Define("Diff3", [&](double diff2, double thetarp) { return diff2 - func2->Eval(thetarp); },
                     {"Diff2", "ThetaLightRP"});
    auto h3 {def.Filter(gateGS, {"ExLegacy"}).Histo2D(mDiffTheta, "ThetaLightRP", "Diff3")};
    FitTH2 fit3 {h3.GetPtr(), npar};
    fit3.Fit();

    // Draw
    auto* c0 {new TCanvas {"c0", "Angle correction"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hEx->DrawClone();
    hExLeg->SetLineColor(kRed);
    hExLeg->DrawClone("same");
    c0->cd(2);
    hExCMLeg->DrawClone("colz");
    c0->cd(3);
    hExRPxLeg->DrawClone("colz");
    gPad->Update();
    // Draw line
    auto* lmin {new TLine {gPad->GetUxmin(), min, gPad->GetUxmax(), min}};
    auto* lmax {new TLine {gPad->GetUxmin(), max, gPad->GetUxmax(), max}};
    for(auto* l : {lmin, lmax})
    {
        l->SetLineWidth(2);
        l->SetLineColor(kMagenta);
        l->Draw("same");
    }
    gROOT->SetSelectedPad(nullptr);
    c0->cd(4);
    hTheoLeg->DrawClone("colz");
    c0->cd(5);
    hDiffRPx->DrawClone("colz");
    c0->cd(6);
    px->Draw();

    auto* c1 {new TCanvas {"c1", "Angle correction"}};
    c1->DivideSquare(6);
    c1->cd(1);
    hTheoThetaRP->DrawClone("colz");
    c1->cd(2);
    hDiffThetaRP->DrawClone("colz");
    c1->cd(3);
    px2->Draw();
    c1->cd(4);
    hDiffThetaLeg->DrawClone("colz");
    c1->cd(5);
    h3->DrawClone("colz");

    // Save on file
    auto file {
        std::make_unique<TFile>(TString::Format("./Outputs/angle_corr_%s_v0.root", isEl ? "side" : "front"), "recreate")};
    func1->Write("func1");
    func2->Write("func2");
}
