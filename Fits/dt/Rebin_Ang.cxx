#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"

#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"
#include "../FitHist.h"

void Rebin_Ang()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")};

    // Book histograms
    ROOT::RDF::RResultPtr<TH2D> hKin {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Exdt, "Ex")};
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 1, 0)};
    ROOT::RDataFrame phase2 {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 2, 0)};

    // Init intervals
    double thetaMin {5.5};
    double thetaMax {14.};
    double thetaStep {1.5};
    int nps {2 + 0}; // 2 nps + 1 contamination
    Angular::Intervals ivs {thetaMin, thetaMax, E796Fit::Exdt, thetaStep, nps};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    // FillPS
    phase.Foreach([&](double thetacm, double ex, double weight) { ivs.FillPS(0, thetacm, ex, weight); },
                  {"theta3CM", "Eex", "weight"});
    phase2.Foreach([&](double thetacm, double ex, double weight) { ivs.FillPS(1, thetacm, ex, weight); },
                   {"theta3CM", "Eex", "weight"});
    ivs.TreatPS(10, 0.2, {0, 1}); // disable smoothing for contamination ps
    ivs.Write("./Outputs/rebin/ivs.root");
    // ivs.Draw();

    // Fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetAllowFreeMean(true, {"v5", "v6", "v7"});
    fitter.SetAllowFreeSigma(true, {"g0"});
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts(true);
    fitter.Write("./Outputs/rebin/counts.root");

    // Interface
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");
    auto peaks {inter.GetPeaks()};
    // Remove contamination
    for(const auto& s : {"v8", "v9", "v10"})
        peaks.erase(std::remove(peaks.begin(), peaks.end(), s), peaks.end());

    // Efficiency
    Interpolators::Efficiency eff;
    for(const auto& peak : peaks)
        eff.Add(peak, gSelector->GetApproxSimuFile("20O", "2H", "3H", inter.GetGuess(peak)), "eff");
    eff.Draw();

    // Set experiment info
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/d_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    ///////////////////////////////////////
    // Cocinha para PID grande
    // xs.TrimX("v0", 12, false);
    // xs.TrimX("v2", 12, false);
    // xs.TrimX("v3", 7);
    // xs.TrimX("v3", 13.5, false);
    // xs.TrimX("v4", 8);
    // xs.TrimX("v5", 8);
    // xs.TrimX("v6", 12.5, false);
    // for(const auto& state : {"v7"})
    // {
    //     xs.TrimX(state, 8);
    //     xs.TrimX(state, 12.5, false);
    // }
    // // xs.TrimX("v4", 13.5, false);
    // // xs.TrimX("v7", 7.5);
    //////////////////////////////////////
    // PID mais pequeno
    for(const auto& state : {"v0", "v1", "v2", "v3", "v4"})
    {
        xs.TrimX(state, 12.5, false);
    }
    xs.TrimX("v3", 6.5);
    xs.TrimX("v4", 6.5);
    xs.TrimX("v5", 8);
    xs.TrimX("v6", 10.5, false);
    xs.TrimX("v7", 6.5);
    xs.TrimX("v7", 12, false);

    xs.Write("./Outputs/rebin/");

    // Comparators!
    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.FillComp();
    inter.FitComp();
    inter.WriteComp("./Outputs/rebin_sfs.root");

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hKin->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();
}
