#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <string>
#include <vector>

#include "../../Selector/Selector.h"
#include "../FitHist.h"

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
    auto hEx {df.Histo1D(E796Fit::Exdt, "Ex")};
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 1, 0)};
    ROOT::RDataFrame phase2 {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 2, 0)};

    // Init intervals
    double thetaCMMin {5.5};
    double thetaCMMax {14};
    double thetaCMStep {1.5};
    int nps {2};
    Angular::Intervals ivs {thetaCMMin, thetaCMMax, E796Fit::Exdt, thetaCMStep, nps};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    // FillPS
    phase.Foreach([&](double thetacm, double ex, double weight) { ivs.FillPS(0, thetacm, ex, weight); },
                  {"theta3CM", "Eex", "weight"});
    phase2.Foreach([&](double thetacm, double ex, double weight) { ivs.FillPS(1, thetacm, ex, weight); },
                   {"theta3CM", "Eex", "weight"});
    ivs.TreatPS();
    // ivs.Draw();

    // Fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetAllowFreeMean(true, {"v6"});
    fitter.Configure("./Outputs/fit_juan.root");
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts(false);

    // Interface
    Fitters::Interface inter;
    inter.Read("./Outputs/interface_juan.root");
    auto peaks {inter.GetPeaks()};

    // Efficiency
    Interpolators::Efficiency eff;
    for(const auto& peak : peaks)
        eff.Add(peak, gSelector->GetApproxSimuFile("20O", "2H", "3H", inter.GetGuess(peak)));
    // eff.Add("g0", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_3H_NumN_0_NumP_0_Ex0_Date_2024_9_27_Time_9_34.root");
    eff.Draw();

    // Set experiment info
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/d_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    xs.TrimX("v0", 13, false);
    xs.Write("./Outputs/", gSelector->GetFlag());

    // Comparators!
    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.DoComp();

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    hEx->DrawClone();
}
