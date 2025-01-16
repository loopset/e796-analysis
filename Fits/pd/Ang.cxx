#include "ROOT/RDataFrame.hxx"

#include "TROOT.h"

#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../FitHist.h"
#include "/media/Data/E796v2/Selector/Selector.h"

void Ang()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "2H")};
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Expd, "Ex")};
    // Phase space
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -2)};

    // Init intervals
    double thetaCMMin {7};
    double thetaCMMax {16};
    double thetaCMStep {1};
    Angular::Intervals ivs {thetaCMMin, thetaCMMax, E796Fit::Expd, thetaCMStep, 1};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    phase.Foreach([&](double thetacm, double ex, double w) { ivs.FillPS(0, thetacm, ex, w); },
                  {"theta3CM", "Eex", "weight"});
    ivs.TreatPS(2);
    // ivs.Draw();

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.Configure("./Outputs/fit_" + gSelector->GetFlag() + ".root");
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts();

    // Interface
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");
    auto peaks {inter.GetPeaks()};

    // Efficiency
    Interpolators::Efficiency eff;
    for(const auto& peak : peaks)
        eff.Add(peak, gSelector->GetApproxSimuFile("20O", "1H", "2H", inter.GetGuess(peak)));
    eff.Draw()->SaveAs("./Outputs/effs.png");

    // Set experiment info
    // Recompute normalization
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/p_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);

    // Comparators!
    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.DoComp();

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hCM->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();
}
