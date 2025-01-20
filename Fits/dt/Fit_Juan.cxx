#include "ROOT/RDataFrame.hxx"

#include "TROOT.h"

#include "FitInterface.h"
#include "FitModel.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include "../../Selector/Selector.h"
#include "../FitHist.h"
void Fit_Juan()
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

    auto hEx {df.Histo1D(E796Fit::Exdt, "Ex")};
    // Phase spaces
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 1, 0)};
    auto hPS {phase.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    hPS->SetNameTitle("hPS", "1n PS");
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr());
    ROOT::RDataFrame phase2 {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 2, 0)};
    auto hPS2 {phase2.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    hPS2->SetNameTitle("hPS", "2n PS");
    Fitters::TreatPS(hEx.GetPtr(), hPS2.GetPtr());

    // Sigma interpolators
    Interpolators::Sigmas sigmas {gSelector->GetSigmasFile("2H", "3H").Data()};

    // Interface for fitting!
    Fitters::Interface inter;
    double sigma {0.364}; // common guess for all states
    inter.AddState("g0", {400, 0, sigma}, "5/2+");
    inter.AddState("g1", {10, 1.4, sigma}, "1/2+");
    inter.AddState("g2", {110, 3.2, sigma}, "3/2+");
    inter.AddState("v0", {60, 4.5, sigma, 0.1}, "3/2-");
    inter.AddState("v1", {60, 6.7, sigma, 0.1}, "?");
    inter.AddState("v2", {60, 7.9, sigma, 0.1}, "?");
    inter.AddState("v3", {60, 8.9, sigma, 0.1}, "?");
    inter.AddState("v4", {60, 11, sigma, 0.1}, "?");
    // inter.AddState("v5", {5, 12.8, sigma, 0.1}, "?");
    inter.AddState("v5", {20, 14.9, sigma, 0.1}, "?");
    inter.AddState("v6", {10, 16, sigma, 0.1}, "?");
    // inter.AddState("v8", {10, 17.5, sigma, 0.1}, "cont");
    inter.AddState("ps0", {0.1});
    inter.AddState("ps1", {0.1});
    inter.EndAddingStates();
    // Wider mean margin
    inter.SetOffsetMeanBounds(0.5);
    // inter.ReadPreviousFit("./Outputs/fit_" + gSelector->GetFlag() + ".root");
    // Eval correct sigma
    inter.EvalSigma(sigmas.GetGraph());
    // Fix all sigmas (3rd parameter, 2nd index of vector)
    inter.SetFixAll(2, true);
    inter.SetBounds("v2", 3, {0, 0.1});
    inter.SetBounds("v3", 3, {0, 0.1});
    inter.SetBounds("v4", 3, {0, 0.1});
    inter.Write("./Outputs/interface_juan.root");

    // Fitting range
    double exmin {-5};
    double exmax {25};
    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS, *hPS2}};
    // Run!
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    "./Outputs/fit_juan.root", "20O(d,t)");
}
