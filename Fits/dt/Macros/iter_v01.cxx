#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"
#include "Rtypes.h"

#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"

#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "FitModel.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <iostream>
#include <string>
#include <vector>

#include "/media/Data/E796v2/Fits/FitHist.h"
#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "/media/Data/E796v2/Selector/Selector.h"

// Define dfs outside as globals

ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")};
ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 1, 0)};
ROOT::RDataFrame phase2 {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 2, 0)};

void fit(double gamma0, double gamma1)
{

    auto hEx {df.Histo1D(E796Fit::Exdt, "Ex")};
    // Phase spaces
    auto hPS {phase.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    hPS->SetNameTitle("hPS", "1n PS");
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr());
    auto hPS2 {phase2.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    hPS2->SetNameTitle("hPS2", "2n PS");
    Fitters::TreatPS(hEx.GetPtr(), hPS2.GetPtr());

    // Sigma interpolators
    Interpolators::Sigmas sigmas {gSelector->GetSigmasFile("2H", "3H").Data()};

    // Interface for fitting!
    Fitters::Interface inter;
    double sigma {0.364}; // common guess for all states
    double offset {0.0};  // != 0 only when using ExLegacy
    inter.AddState("g0", {400, 0 - offset, sigma}, "5/2+");
    inter.AddState("g1", {10, 1.4 - offset, sigma}, "1/2+");
    inter.AddState("g2", {110, 3.2 - offset, sigma}, "(1/2,3/2)-");
    inter.AddState("v0", {60, 4.5 - offset, sigma, gamma0}, "3/2-");
    inter.AddState("v1", {60, 6.7 - offset, sigma, gamma1}, "?");
    inter.AddState("v2", {60, 7.9 - offset, sigma, 0.1}, "?");
    inter.AddState("v3", {60, 8.9 - offset, sigma, 0.1}, "?");
    inter.AddState("v4", {60, 11 - offset, sigma, 0.1}, "?");
    inter.AddState("v5", {40, 12.2 - offset, sigma, 0.1}, "?");
    inter.AddState("v6", {40, 13.8 - offset, sigma, 0.1}, "?");
    inter.AddState("v7", {20, 14.9 - offset, sigma, 0.1}, "?");
    inter.AddState("v8", {20, 16. - offset, sigma, 0.}, "Cont0");
    inter.AddState("ps0", {0.1});
    inter.AddState("ps1", {0.1});
    inter.EndAddingStates();
    // Wider mean margin
    inter.SetOffsetMeanBounds(0.5);
    // inter.ReadPreviousFit("../Outputs/fit_" + gSelector->GetFlag() + ".root");
    // Eval correct sigma
    inter.EvalSigma(sigmas.GetGraph());
    // Fix gammas here
    inter.SetFix("v0", 3, true);
    inter.SetFix("v1", 3, true);
    // this value has been obtained from the larger PID fit
    inter.SetFixAll(2, true);
    inter.SetBounds("v2", 3, {0, 0.1});
    inter.SetBounds("v3", 3, {0, 0.1});
    inter.SetBounds("v5", 3, {0, 0.2});
    inter.SetBounds("v6", 3, {0, 0.1});
    inter.SetBounds("v7", 3, {0, 0.1});
    for(const auto& s : {"v8"})
    {
        // inter.SetFix(s, 1, true);
        inter.SetFix(s, 3, true);
    }
    inter.Write(TString::Format("./Outputs/Iter/inter_%.2f_%.2f.root", gamma0, gamma1).Data());

    // Fitting range
    double exmin {-5};
    double exmax {22};
    // Model
    std::vector<TH1D> hPSs {*hPS, *hPS2};
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), hPSs};
    // Run!
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    TString::Format("./Outputs/Iter/fit_%.2f_%.2f.root", gamma0, gamma1).Data(), "20O(d,t)");
}
void ang(double gamma0, double gamma1, bool rebin)
{
    // Init intervals
    double thetaMin {5.5};
    double thetaMax {14.};
    double thetaStep {rebin ? 1.5 : 1.};
    int nps {static_cast<int>(2)};
    Angular::Intervals ivs {thetaMin, thetaMax, E796Fit::Exdt, thetaStep, nps};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    // FillPS
    phase.Foreach([&](double thetacm, double ex, double weight) { ivs.FillPS(0, thetacm, ex, weight); },
                  {"theta3CM", "Eex", "weight"});
    phase2.Foreach([&](double thetacm, double ex, double weight) { ivs.FillPS(1, thetacm, ex, weight); },
                   {"theta3CM", "Eex", "weight"});
    ivs.TreatPS(10, 0.2, {0, 1}); // disable smoothing for contamination ps

    // Fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetAllowFreeMean(true, {"v5", "v7"});
    fitter.SetAllowFreeSigma(true, {"g0"});
    // fitter.SetAllowFreeGamma(true, {"v1"});
    fitter.Configure(TString::Format("./Outputs/Iter/fit_%.2f_%.2f.root", gamma0, gamma1).Data());
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts(true);

    // Interface
    Fitters::Interface inter;
    inter.Read(TString::Format("./Outputs/Iter/inter_%.2f_%.2f.root", gamma0, gamma1).Data());
    auto peaks {inter.GetPeaks()};
    // Remove contamination
    for(const auto& s : {"v8", "v9", "v10"})
        peaks.erase(std::remove(peaks.begin(), peaks.end(), s), peaks.end());

    // Efficiency
    Interpolators::Efficiency eff;
    for(const auto& peak : peaks)
        eff.Add(peak, gSelector->GetApproxSimuFile("20O", "2H", "3H", inter.GetGuess(peak)), "eff");
    eff.Scale(0.95); // Calculated experimental rec efficiency from paper (underestimated)

    // Set experiment info
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"/media/Data/E796v2/Fits/norms/d_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    xs.TrimX("v0", 12.75, false);
    xs.TrimX("v3", 7.5);
    xs.TrimX("v3", 13.5, false);
    xs.TrimX("v4", 7.5);
    xs.TrimX("v4", 13.5, false);
    xs.TrimX("v7", 7);
    // xs.TrimX("v8", 6.5);
    // xs.TrimX("v8", 11.5, false);

    // Comparators!
    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    std::string pwd {gSystem->pwd()}; // workaround bc input for cmp is relative to dt/ dir
    gSystem->cd("/media/Data/E796v2/Fits/dt/");
    inter.ReadCompConfig("./comps.conf");
    inter.FillComp();
    inter.FitComp();
    gSystem->cd(pwd.c_str());
    inter.WriteComp(
        TString::Format("./Outputs/Iter/sfs_%.2f_%.2f%s.root", gamma0, gamma1, rebin ? "_rebin" : "").Data());
}

void iter_v01()
{
    gROOT->SetBatch();

    std::vector<double> gamma0, gamma1;
    for(double g = 0; g < 1.5; g += 0.1)
    {
        gamma0.push_back(g);
        gamma1.push_back(g);
    }

    // Run!
    for(const auto& g0 : gamma0)
    {
        for(const auto& g1 : gamma1)
        {
            std::cout << "::::::::::::::::::::::::::::::::::::::::::::::::::" << '\n';
            // If it is already calcualted, skip it
            auto test {TString::Format("./Outputs/Iter/fit_%.2f_%.2f.root", g0, g1)};
            // if(!gSystem->AccessPathName(test))
            // {
            //     std::cout << "Skipping already done (g0, g1) : (" << g0 << ", " << g1 << ")" << '\n';
            //     continue;
            // }
            std::cout << "Gamma0 : " << g0 << " Gamma 1 : " << g1 << '\n';
            fit(g0, g1);
            // ang(g0, g1, false);
            // ang(g0, g1, true);
        }
    }
}
