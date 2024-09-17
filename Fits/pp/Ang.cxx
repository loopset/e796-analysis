#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../PostAnalysis/Utils.cxx"
#include "../FitHist.h"

void Ang()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Final_Tree", E796Utils::GetFileName(2, "20O", "1H", "1H", true)};

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Exdd, "Ex")};

    // Init intervals
    double thetaCMMin {18};
    double thetaCMMax {25};
    double thetaCMStep {1};
    Angular::Intervals ivs {thetaCMMin, thetaCMMax, E796Fit::Exdt, thetaCMStep};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.Configure("./Outputs/fit_pp.root", {});
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts();

    // Read efficiency files
    std::vector<std::string> peaks {"g0"};
    std::vector<std::string> effFiles {
        "/media/Data/E796v2/Simulation/Outputs/e796_beam_20O_target_1H_light_1H_Eex_0.00_nPS_0_pPS_0.root",
        "/media/Data/E796v2/Simulation/Outputs/e796_beam_20O_target_1H_light_1H_Eex_1.67_nPS_0_pPS_0.root",
    };
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
        eff.Add(peaks[p], effFiles[p]);
    // Draw to check is fine
    eff.Draw(true);

    // Set experiment info
    // Nt is different: now H from C4H10
    // remember we are using an effective length!
    PhysUtils::Experiment exp {4.5625e20, 279932, 30000};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);

    // For gs
    Angular::Comparator comp {"g0 = 0^{+} g.s", xs.Get("g0")};
    comp.Add("KD", "./Inputs/g0_KD/fort.201");
    comp.Fit();
    comp.Draw("g.s", true, true);

    // // For g1
    // Angular::Comparator comp1 {"g1 = 2^{+} @ 1.67 MeV", xs.Get("g1")};
    // // comp2.Add("l = 1", "./Inputs/g2/l_1/21.g2");
    // // comp2.Add("l = 2", "./Inputs/g2/l_2/21.g2");
    // // comp2.Fit(thetaCMMin, thetaCMMax);
    // comp1.Draw();

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hCM->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();
}
