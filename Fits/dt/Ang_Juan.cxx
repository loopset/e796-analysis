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

void Ang_Juan()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame d {"yield_tree", "/media/Data/E796v2/RootFiles/Old/FitJuan/20O_dt_19O_21_Feb_23_v1.root"};
    // Apply mass cuts from Juan
    auto applyMassCuts = [&](double AmassH, double thetaCM, double Ex)
    {
        // fits to mass gaussians
        // A = 2
        double A2mass {2.05609 - 0.0328848 * Ex + 0.00288087 * Ex * Ex};
        double S2mass {0.251386 + 0.0171722 * Ex - 0.00191928 * Ex * Ex};
        // A = 3
        double A3mass {3.16142 - 0.034846 * Ex + 0.000415012 * Ex * Ex};
        double S3mass {0.306103 - 0.00590731 * Ex - 0.000331309 * Ex * Ex};

        int kA2 {1};
        int kA3 {2};
        // Mass cut
        bool A2cut {(A2mass - kA2 * S2mass < AmassH) && (AmassH < A2mass + kA2 * S2mass)};
        bool A3cut {(A3mass - kA3 * S3mass < AmassH) && (AmassH < A3mass + kA3 * S3mass)};
        // Cut in thetaCM (only centered detectors)
        // bool thetaCMcut {4. <= thetaCM && thetaCM <= 13};
        bool thetaCMcut {true};
        // Ex less than 10 MeV
        // bool Excut {Ex < 10};
        bool Excut {true};

        return (A3cut && thetaCMcut && Excut);
    };
    auto df {d.Filter(applyMassCuts, {"Amass_Hlike", "ThetaCM", "Ex"})};

    // Book histograms
    int nbins {100};
    double hmin {-5};
    double hmax {25};
    auto hEx {df.Histo1D(
        {"hEx", TString::Format(";E_{x} [MeV];Counts / %.0f keV", (hmax - hmin) / nbins * 1E3), nbins, hmin, hmax},
        "Ex")};
    // Read PS
    ROOT::RDataFrame phase {"simulated_tree", "/media/Data/E796v2/RootFiles/Old/FitJuan/"
                                              "20O_and_2H_to_3H_NumN_1_NumP_0_Ex0_Date_2022_11_29_Time_16_35.root"};
    auto hPS {phase.Histo1D({"hPS", "PS 1n;E_{x} [MeV]", nbins, hmin, hmax}, "Ex_cal", "Weight_sim")};
    // Format phase space
    hPS->Smooth(20);
    // Scale it
    auto intEx {hEx->Integral()};
    auto intPS {hPS->Integral()};
    double factor {0.15};
    hPS->Scale(factor * intEx / intPS);

    // Init intervals
    double thetaCMMin {8};
    double thetaCMMax {14};
    double thetaCMStep {1};
    Angular::Intervals ivs {thetaCMMin, thetaCMMax, {"hEx", "(d, t)", nbins, hmin, hmax}, thetaCMStep};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    // ivs.Draw();

    // Init fitter
    // Set range
    double exMin {hmin};
    double exMax {10};
    Angular::Fitter fitter {&ivs};
    fitter.Configure("./Outputs/fit_juan.root", {*hPS});
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts();

    // Read efficiency files
    std::vector<std::string> peaks {"g0", "g2", "g3"};
    std::vector<std::string> effFiles {
        "/media/Data/E796v2/Simulation/Outputs/e796_beam_20O_target_2H_light_3H_Eex_0.00_nPS_0_pPS_0.root",
        "/media/Data/E796v2/Simulation/Outputs/e796_beam_20O_target_2H_light_3H_Eex_3.24_nPS_0_pPS_0.root",
        "/media/Data/E796v2/Simulation/Outputs/e796_beam_20O_target_2H_light_3H_Eex_4.40_nPS_0_pPS_0.root",
    };
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
        eff.Add(peaks[p], effFiles[p]);
    // Draw to check is fine
    eff.Draw();
    // Set experiment info
    PhysUtils::Experiment exp {8.2125e20, 279932, 30000};
    // std::cout << "Nb : " << exp.GetNb() << '\n';
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    // xs.Draw();

    // For gs
    Angular::Comparator comp {"g.s", xs.Get("g0")};
    comp.Add("l = 2 Franck", "./Inputs/gs/Franck/gs.xs");
    comp.Fit(thetaCMMin, thetaCMMax);
    comp.DrawTheo();
    comp.Draw();
    comp.ScaleToExp("l = 2 Franck", 3.43, fitter.GetIgCountsGraph("g0"), eff.GetTEfficiency("g0"));

    // For g2 @ 3.2 MeV
    Angular::Comparator comp2 {"g2 = 1/2^{-} @ 3.2 MeV", xs.Get("g2")};
    comp2.Add("l = 1", "./Inputs/g2/l_1/21.g2");
    comp2.Add("l = 2", "./Inputs/g2/l_2/21.g2");
    comp2.Fit(thetaCMMin, thetaCMMax);
    comp2.Draw();
    
    // For g3 @ 4.7 MeV
    Angular::Comparator comp3 {"g3 = 3/2^{-} @ 4.58 MeV", xs.Get("g3")};
    comp3.Add("l = 1", "./Inputs/g3/21.g3");
    comp3.Fit(thetaCMMin, thetaCMMax);
    comp3.Draw();

    // // plotting
    // auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    // hEx->DrawClone();
}
