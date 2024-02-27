#include "ROOT/RDataFrame.hxx"

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

void Ang()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {
        "Final_Tree", "/media/Data/E796v2/PostAnalysis/RootFiles/Legacy/tree_beam_20O_target_1H_light_2H_front.root"};

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    int nbins {100};
    double hmin {-10};
    double hmax {6};
    auto hEx {df.Histo1D(
        {"hEx", TString::Format(";E_{x} [MeV];Counts / %.0f keV", (hmax - hmin) / nbins * 1E3), nbins, hmin, hmax},
        "Ex")};

    // Init intervals
    double thetaCMMin {6};
    double thetaCMMax {16};
    double thetaCMStep {1};
    Angular::Intervals ivs {thetaCMMin, thetaCMMax, {"hEx", "(d, t)", nbins, hmin, hmax}, thetaCMStep};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    // ivs.Draw();

    // Init fitter
    // Set range
    double exMin {hmin};
    double exMax {5};
    Angular::Fitter fitter {&ivs, exMin, exMax};
    fitter.Configure("./Outputs/fit_pd.root", {});
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts();

    // Read efficiency files
    std::vector<std::string> peaks {"g0", "g1", "g2"};
    std::vector<std::string> effFiles {
        "/media/Data/E796v2/Simulation/Outputs/e796_beam_20O_target_1H_light_2H_Eex_0.00_nPS_0_pPS_0.root",
        "/media/Data/E796v2/Simulation/Outputs/e796_beam_20O_target_1H_light_2H_Eex_1.47_nPS_0_pPS_0.root",
        "/media/Data/E796v2/Simulation/Outputs/e796_beam_20O_target_1H_light_2H_Eex_3.24_nPS_0_pPS_0.root",
    };
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
        eff.Add(peaks[p], effFiles[p]);
    // Draw to check is fine
    eff.Draw(true);
    // Set experiment info
    // Nt is different: now H from C4H10
    PhysUtils::Experiment exp {4.554e19, 279932, 30000};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    xs.Draw();

    // For gs
    Angular::Comparator comp {"g.s", xs.Get("g0")};
    // comp.Add("l = 2", "./Inputs/gs/21.gs");
    comp.Add("l = 2", "./Inputs/gs_new/21.gs");
    comp.Fit(thetaCMMin, thetaCMMax);
    comp.DrawTheo();
    comp.Draw();
    // Scale to theo
    auto* g0Counts {fitter.GetIgCountsGraph("g0")};
    comp.ScaleToExp("l = 2", 3.43, g0Counts, eff.GetTEfficiency("g0"));

    // First excited state
    Angular::Comparator comp1 {"(1/2)^{+}", xs.Get("g1")};
    comp1.Add("l = 0", "./Inputs/g1/21.g1");
    comp1.Fit(thetaCMMin, thetaCMMax);
    comp1.Draw();
    // comp1.DrawTheo();

    // Second excited state
    Angular::Comparator comp2 {"1/2^{-} @ 3.2 MeV", xs.Get("g2")};
    comp2.Add("l = 1", "./Inputs/g2/21.g2");
    comp2.Fit(thetaCMMin, thetaCMMax);
    comp2.Draw();
    // Scale to theo
    comp2.ScaleToExp("l = 1", 3.43, fitter.GetIgCountsGraph("g2"), eff.GetTEfficiency("g2"));

    // Debug efficiency
    std::vector<Interpolators::Efficiency> veff;
    Interpolators::Efficiency deff;
    std::vector<std::string> dpeaks {"gs_all", "gs_1", "gs_2"};
    std::vector<std::string> dfiles {
        "/media/Data/E796v2/Simulation/Outputs/Eff_study/p_d_gs_all.root",
        "/media/Data/E796v2/Simulation/Outputs/Eff_study/p_d_gs_1.root",
        "/media/Data/E796v2/Simulation/Outputs/Eff_study/p_d_gs_2.root",
    };
    for(int i = 0; i < dpeaks.size(); i++)
    {
        deff.Add(dpeaks[i], dfiles[i]);
        veff.push_back({});
        veff.back().Add("g0", dfiles[i]);
    }
    deff.Draw();
    // Do calculation
    std::vector<Angular::DifferentialXS> dxs;
    std::vector<Angular::Comparator> dcomp;
    for(int i = 0; i < dpeaks.size(); i++)
    {
        dxs.push_back(Angular::DifferentialXS {&ivs, &fitter, &veff[i], &exp});
        dxs.back().DoFor({"g0"});
        // and compare
        dcomp.push_back(Angular::Comparator {dpeaks[i], dxs.back().Get("g0")});
        dcomp.back().Add("l = 2", "./Inputs/gs_new/21.gs");
        dcomp.back().Fit(thetaCMMin, thetaCMMax);
        dcomp.back().Draw();
    }
    // // plotting
    // auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    // c0->DivideSquare(2);
    // c0->cd(1);
    // hCM->DrawClone("colz");
    // c0->cd(2);
    // hEx->DrawClone();
    // c0->cd(3);
    // xs.Get("g0")->Draw("apl");
    // hCM->DrawClone("colz");
}
