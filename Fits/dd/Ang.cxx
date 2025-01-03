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

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "2H")};

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Exdd, "Ex")};

    // Init intervals
    double thetaCMMin {15};
    double thetaCMMax {22};
    double thetaCMStep {1};
    Angular::Intervals ivs {thetaCMMin, thetaCMMax, E796Fit::Exdd, thetaCMStep};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts();

    // Read efficiency files
    std::vector<std::string> peaks {"g0", "g1", "g2", "g3"};
    std::vector<std::string> effFiles {
        gSelector->GetSimuFile("20O", "2H", "2H", 0).Data(),
        gSelector->GetSimuFile("20O", "2H", "2H", 1.67).Data(),
        gSelector->GetSimuFile("20O", "2H", "2H", 4.1).Data(),
        gSelector->GetSimuFile("20O", "2H", "2H", 5.52).Data(),
    };
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
        eff.Add(peaks[p], effFiles[p]);
    // Draw to check is fine
    eff.Draw(true);

    // Set experiment info
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/d_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    xs.TrimX("g1", 17);
    xs.TrimX("g2", 16.2);
    xs.TrimX("g3", 16.2);
    xs.Write("./Outputs/");

    // For gs
    Angular::Comparator comp {"g0 = 0^{+} g.s", xs.Get("g0")};
    comp.Add("Daeh", "./Inputs/g0_Daehnick/fort.201");
    comp.Add("Haixia", "./Inputs/g0_Haixia/fort.201");
    // comp.Add("Fit", "./Inputs/s0/fort.201");
    comp.Fit();
    comp.Draw("", true);

    // For g1
    Angular::Comparator comp1 {"g1 = 2^{+} @ 1.67 MeV", xs.Get("g1")};
    comp1.Add("B(E2) = 28.1 E. Khan", "./Inputs/g1_Haixia/fort.202");
    comp1.Fit();
    comp1.Draw();

    // For g2
    Angular::Comparator comp2 {"g2 = 2^{+} @ 4 MeV", xs.Get("g2")};
    comp2.Add("B(E2) = 28.1 E.Khan", "./Inputs/g2_Haixia/fort.202");
    comp2.Fit();
    comp2.Draw();
    
    // For g3
    Angular::Comparator comp3 {"g3 = 3^{-} @ 5.6 MeV", xs.Get("g2")};
    comp3.Add("B(E3) = 882 E. Khan", "./Inputs/g3_Haixia/fort.202");
    comp3.Fit();
    comp3.Draw();

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hCM->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();
}
