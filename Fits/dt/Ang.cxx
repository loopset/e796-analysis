#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include "AngFitter.h"
#include "AngIntervals.h"

#include "../../PostAnalysis/HistConfig.h"

void Ang()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {
        "Final_Tree", "/media/Data/E796v2/PostAnalysis/RootFiles/Legacy/tree_beam_20O_target_2H_light_3H_front.root"};

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    int nbins {100};
    double hmin {-5};
    double hmax {25};
    auto hEx {df.Histo1D(
        {"hEx", TString::Format(";E_{x} [MeV];Counts / %.0f keV", (hmax - hmin) / nbins * 1E3), nbins, hmin, hmax},
        "Ex")};
    // Read PS
    ROOT::RDataFrame phase {"simulated_tree", "/media/Data/E796v2/RootFiles/Old/FitJuan/"
                                              "20O_and_2H_to_3H_NumN_1_NumP_0_Ex0_Date_2022_11_29_Time_16_35.root"};
    auto hPS {phase.Histo1D({"hPS", "PS 1n;E_{x} [MeV]", nbins, hmin, hmax}, "Ex_cal")};
    // Format phase space
    hPS->Smooth(20);
    // Scale it
    auto intEx {hEx->Integral()};
    auto intPS {hPS->Integral()};
    double factor {0.15};
    hPS->Scale(factor * intEx / intPS);

    // Init intervals
    double thetaCMMin {3};
    double thetaCMMax {15};
    double thetaCMStep {2};
    Angular::Intervals ivs {thetaCMMin, thetaCMMax, {"hEx", "(d, t)", nbins, hmin, hmax}, thetaCMStep};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    // ivs.Draw();

    // Init fitter
    // Set range
    double exMin {hmin};
    double exMax {10};
    Angular::Fitter fitter {ivs, exMin, exMax};
    fitter.Configure("./Outputs/fit_dt.root", {*hPS});
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts();

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hCM->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();
    // hCM->DrawClone("colz");
}
