#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"
#include "../FitHist.h"

void Ang()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "1H")};
    // Phase space deuton breakup
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -1, 0)};

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Expp, "Ex")};

    // Init intervals
    double thetaCMMin {18};
    double thetaCMMax {25};
    double thetaCMStep {0.5};
    Angular::Intervals ivs {thetaCMMin, thetaCMMax, E796Fit::Expp, thetaCMStep, 1};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    phase.Foreach([&](double thetacm, double ex, double w) { ivs.FillPS(0, thetacm, ex, w); },
                  {"theta3CM", "Eex", "weight"});
    ivs.TreatPS(2);

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts();

    // Read efficiency files
    std::vector<std::string> peaks {"g0", "g1"};
    std::vector<std::string> effFiles {
        gSelector->GetSimuFile("20O", "1H", "1H", 0).Data(),
        gSelector->GetSimuFile("20O", "1H", "1H", 1.67).Data(),
    };
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
        eff.Add(peaks[p], effFiles[p]);
    // Draw to check is fine
    eff.Draw(true);

    // Recompute normalzation
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/p_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    xs.TrimX("g0", 24.5, false);

    // For gs
    Angular::Comparator comp {"g0 = 0^{+} g.s", xs.Get("g0")};
    comp.Add("KD", "./Inputs/g0_KD/fort.201");
    comp.Add("CH89", "./Inputs/g0_CH89/fort.201");
    comp.Fit();
    comp.Draw("g.s", true, true)->SaveAs("./Outputs/pp_0.png");

    // For g1
    Angular::Comparator comp1 {"g1 = 2^{+} @ 1.67 MeV", xs.Get("g1")};
    comp1.Add("B(E2) E.Khan", "./Inputs/g1_KD/fort.202");
    comp1.Fit();
    comp1.Draw()->SaveAs("./Outputs/pp_1.png");

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hCM->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();
}
