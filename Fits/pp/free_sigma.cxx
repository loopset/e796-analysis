#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TROOT.h"

#include "AngFitter.h"
#include "AngIntervals.h"

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"
#include "../FitHist.h"

void free_sigma(bool isLab = false)
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "1H")};
    // Phase space deuton breakup
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
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    phase.Foreach([&](double thetacm, double ex, double w) { ivs.FillPS(0, thetacm, ex, w); },
                  {"theta3CM", "Eex", "weight_trans"});
    ivs.TreatPS(4);
    // ivs.FitPS("pol6");
    // ivs.ReplacePSWithFit();
    ivs.Draw();

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetManualRange(-4, 12);
    // fitter.SetAllowFreeMean(true, {"g0"});
    // fitter.SetFreeMeanRange(0.1);
    fitter.SetAllowFreeSigma(true, {"g0"});
    fitter.SetFreeSigmaRange(0.25);
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitter.Run();
    fitter.Draw();
    fitter.DrawCounts();


    // Sigma per interval
    auto* gsigmas {new TGraphErrors};
    gsigmas->SetTitle("Sigmas;#theta_{CM} [#circ];#sigma [MeV]");
    for(int i = 0; i < ivs.GetSize(); i++)
    {
        auto x {ivs.GetCenter(i)};
        auto y {fitter.GetTFitResult(i).Parameter(2)};
        gsigmas->AddPoint(x, y);
    }

    ////////////////// Same for simulation
    ROOT::RDataFrame simu {"SimulationTTree", "../../Simulation/Outputs/juan_RPx/tree_20O_1H_1H_0.00_nPS_0_pPS_0.root"};
    Angular::Intervals simivs {thetaMin, thetaMax, E796Fit::Expp, thetaStep, 1};
    // Fill
    simu.Foreach([&](double thetacm, double ex) { simivs.Fill(thetacm, ex); }, {"theta3CM", "Eex"});
    simivs.Draw();

    Angular::Fitter fitsimu {&simivs};
    fitsimu.SetManualRange(-4, 12);
    // fitsimu.SetAllowFreeMean(true, {"g0"});
    // fitsimu.SetFreeMeanRange(0.1);
    fitsimu.SetAllowFreeSigma(true, {"g0"});
    fitsimu.SetFreeSigmaRange(0.25);
    fitsimu.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitsimu.Run();
    fitsimu.Draw();

    auto* gsimu {new TGraphErrors};
    gsimu->SetTitle("Sigmas;#theta_{CM} [#circ];#sigma [MeV]");
    for(int i = 0; i < ivs.GetSize(); i++)
    {
        auto x {ivs.GetCenter(i)};
        auto y {fitsimu.GetTFitResult(i).Parameter(2)};
        gsimu->AddPoint(x, y);
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "free sigma canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    gsigmas->Draw("a*l");
    c0->cd(2);
    gsimu->SetLineColor(kRed);
    gsimu->Draw("a*l");
}
