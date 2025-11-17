#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TROOT.h"
#include "TString.h"

#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngGlobals.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"
#include "../FitHist.h"

void Ang(bool isLab = false)
{
    if(isLab)
        Angular::ToggleIsLab();

    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "1H")};
    // Phase space deuton breakup
    // ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -1, 0)};
    ROOT::RDataFrame phase {"SimulationTTree",
                            "../../Simulation/Macros/Breakup/Outputs/d_breakup_trans.root"}; // set weight_trans

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Expp, "Ex")};

    // Init intervals
    double thetaMin {isLab ? 75. : 18};
    double thetaMax {isLab ? 82. : 24};
    double thetaStep {isLab ? 1. : 1};
    Angular::Intervals ivs {thetaMin, thetaMax, E796Fit::Expp, thetaStep, 1};
    // Fill
    if(!isLab)
    {
        df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
        phase.Foreach([&](double thetacm, double ex, double w) { ivs.FillPS(0, thetacm, ex, w); },
                      {"theta3CM", "Eex", "weight_trans"});
    }
    else
    {
        df.Foreach([&](float thetalab, double ex) { ivs.Fill(thetalab, ex); }, {"fThetaLight", "Ex"});
        phase.Foreach([&](double thetalab, double ex, double w) { ivs.FillPS(0, thetalab, ex, w); },
                      {"theta3Lab", "Eex", "weight_trans"});
    }
    ivs.TreatPS(4);
    // ivs.FitPS("pol6");
    // ivs.ReplacePSWithFit();
    ivs.Draw();
    if(!isLab)
        ivs.Write("./Outputs/ivs.root");

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetManualRange(-4, 12);
    // fitter.SetAllowFreeMean(true, {"g0"});
    // fitter.SetFreeMeanRange(0.1);
    // fitter.SetAllowFreeSigma(true, {"g0"});
    // fitter.SetFreeSigmaRange(0.1);
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitter.Run();
    fitter.Draw();
    fitter.DrawCounts();
    if(!isLab)
        fitter.Write("./Outputs/fitter.root");

    // Sigma graph for g.s
    auto* gsigma {new TGraphErrors};
    gsigma->SetTitle("g.s #sigma with #theta_{CM};#theta_{CM} [#circ];sigma g.s [MeV]");
    for(int i = 0; i < ivs.GetSize(); i++)
    {
        auto x {ivs.GetCenter(i)};
        auto y {fitter.GetTFitResults()[i].Parameter(2)};
        auto uy {fitter.GetTFitResults()[i].Error(2)};
        gsigma->AddPoint(x, y);
    }

    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");
    // inter.Print();
    auto peaks {inter.GetKeys()};

    // Efficiency
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
        eff.Add(peaks[p], gSelector->GetSimuFile("20O", "1H", "1H", inter.GetGuess(peaks[p])).Data(),
                isLab ? "effLab" : "eff");
    // Draw to check is fine
    eff.Draw(true)->SaveAs("./Outputs/effs.png");

    // Recompute normalzation
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/p_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    if(!isLab)
    {
        xs.TrimX("g0", 18.5, true);
        xs.TrimX("g0", 24.5, false);
        xs.Write("./Outputs/");
    }

    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.DoComp();
    inter.GetComp("g0")->ScaleToExp("BG", &exp, fitter.GetIgCountsGraph("g0"), eff.GetTEfficiency("g0"));
    if(!isLab)
        inter.WriteComp("./Outputs/sfs.root");

    auto comp {inter.GetComp("g0")};
    comp->DrawSFfromIntegral(true);

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hCM->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();
    c0->cd(3);
    gsigma->SetLineWidth(2);
    gsigma->SetMarkerStyle(24);
    gsigma->Draw("apl");

    if(!isLab)
        gSelector->SendToWebsite("pp.root", gROOT->GetListOfCanvases());
}
