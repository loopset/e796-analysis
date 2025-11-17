#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
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

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "2H")};

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Exdd, "Ex")};
    // Phase space 19O
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, 1, 0)};
    auto hPS {phase.Histo1D(E796Fit::Exdd, "Eex", "weight")};

    // Init intervals
    double thetaMin {isLab ? 70. : 15};
    double thetaMax {isLab ? 84. : 22};
    double thetaStep {isLab ? 2.5 : 1};
    Angular::Intervals ivs {thetaMin, thetaMax, E796Fit::Exdd, thetaStep, 1};
    // Fill
    if(!isLab)
    {
        df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
        phase.Foreach([&](double thetacm, double ex, double w) { ivs.FillPS(0, thetacm, ex, w); },
                      {"theta3CM", "Eex", "weight"});
    }
    else
    {
        df.Foreach([&](float thetalab, double ex) { ivs.Fill(thetalab, ex); }, {"fThetaLight", "Ex"});
        phase.Foreach([&](double thetalab, double ex, double w) { ivs.FillPS(0, thetalab, ex, w); },
                      {"theta3Lab", "Eex", "weight"});
    }
    ivs.TreatPS(2);
    if(!isLab)
    {
        ivs.FitPS("pol4");
        ivs.ReplacePSWithFit();
    }
    ivs.Draw();

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetAllowFreeMean(true);
    // fitter.SetFreeMeanRange(0.1);
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitter.Run();
    fitter.Draw();
    fitter.DrawCounts();

    // Interface
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");
    auto peaks {inter.GetKeys()};

    // Efficiency
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
    {
        const auto& peak {peaks[p]};
        eff.Add(peak, gSelector->GetSimuFile("20O", "2H", "2H", inter.GetGuess(peak)).Data(), isLab ? "effLab" : "eff");
    }
    // Draw to check is fine
    eff.Draw();

    // Set experiment info
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/d_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    if(!isLab)
    {
        xs.TrimX("g1", 16.75);
        xs.TrimX("g1", 21, false);
        xs.TrimX("g2", 16.2);
        xs.TrimX("g3", 17.8);
        xs.Write("./Outputs/");
    }

    // Init comparators!
    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.DoComp();
    if(!isLab)
        inter.WriteComp("./Outputs/sfs.root");
    inter.GetComp("g0")->DrawSFfromIntegral(true);

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hCM->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();

    if(!isLab)
        gSelector->SendToWebsite("dd.root", gROOT->GetListOfCanvases());
}
