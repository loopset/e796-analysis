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

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")};

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Exdt, "Ex")};
    // Read PS
    ROOT::RDataFrame phase {"simulated_tree", "/media/Data/E796v2/RootFiles/Old/FitJuan/"
                                              "20O_and_2H_to_3H_NumN_1_NumP_0_Ex0_Date_2022_11_29_Time_16_35.root"};
    auto hPS {phase.Histo1D(E796Fit::Exdt, "Ex_cal", "Weight_sim")};
    hPS->SetNameTitle("hPS", "1n PS");
    // Format phase space
    hPS->Smooth(20);
    // Scale it
    auto intEx {hEx->Integral()};
    auto intPS {hPS->Integral()};
    double factor {0.10};
    hPS->Scale(factor * intEx / intPS);

    // Init intervals
    double thetaCMMin {8};
    double thetaCMMax {14};
    double thetaCMStep {1.5};
    Angular::Intervals ivs {thetaCMMin, thetaCMMax, E796Fit::Exdt, thetaCMStep};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    // ivs.Draw();

    // Init fitter
    // Set range
    Angular::Fitter fitter {&ivs};
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data(), {*hPS});
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts();

    // Read efficiency files
    std::vector<std::string> peaks {"g0", "g2", "g3", "g4"};
    std::vector<std::string> effFiles {
        // "/media/Data/E796v2/Simulation/Outputs/Old/e796_beam_20O_target_2H_light_3H_Eex_0.00_nPS_0_pPS_0.root",
        gSelector->GetSimuFile("20O", "2H", "3H", 0).Data(),
        gSelector->GetSimuFile("20O", "2H", "3H", 3.24).Data(),
        gSelector->GetSimuFile("20O", "2H", "3H", 4.40).Data(),
        gSelector->GetSimuFile("20O", "2H", "3H", 6.90).Data(),
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
    // xs.Draw();

    // For gs
    Angular::Comparator comp {"g0 = 5/2^{+} g.s", xs.Get("g0")};
    // comp.Add("l = 2", "./Inputs/gs/l_2/21.gs");
    // comp.Add("l = 2 Ramus", "./Inputs/gs/l_2_Ramus/21.gs");
    // comp.Add("l = 2 Juan", "./Inputs/gs/Juan/GS/OP1_1/21.XS");
    comp.Add("l = 2 Franck", "./Inputs/gs/Franck/gs.xs");
    comp.Add("l = 2 E_{x} = 96 keV", "./Inputs/gs/cl_2/21.c");
    comp.Fit(thetaCMMin, thetaCMMax);
    comp.Draw();
    // comp.ScaleToExp("l = 2 Franck", 3.43, fitter.GetIgCountsGraph("g0"), eff.GetTEfficiency("g0"));

    // For g2
    Angular::Comparator comp2 {"g2 = 1/2^{-} @ 3.2 MeV", xs.Get("g2")};
    comp2.Add("l = 1", "./Inputs/g2/l_1/21.g2");
    comp2.Add("l = 2", "./Inputs/g2/l_2/21.g2");
    comp2.Fit(thetaCMMin, thetaCMMax);
    comp2.Draw();
    // comp2.ScaleToExp("l = 1", 3.43, fitter.GetIgCountsGraph("g2"), eff.GetTEfficiency("g2"));

    // For g3
    Angular::Comparator comp3 {"g3 = 3/2^{-} @ 4.58 MeV", xs.Get("g3")};
    comp3.Add("l = 1", "./Inputs/g3/21.g3");
    comp3.Fit(thetaCMMin, thetaCMMax);
    comp3.Draw();

    // For g4
    Angular::Comparator comp4 {"g4 = ? @ 6.68 MeV", xs.Get("g4")};
    comp4.Draw();
    // how do I compute the 2fnr without a guess of Jpi?


    // // Debug efficiency
    // std::vector<Interpolators::Efficiency> veff;
    // Interpolators::Efficiency deff;
    // std::vector<std::string> dpeaks {"gs_all", "gs_1", "gs_2"};
    // std::vector<std::string> dfiles {
    //     "/media/Data/E796v2/Simulation/Outputs/Eff_study/d_t_gs_all.root",
    //     // "/media/Data/E796v2/Simulation/Outputs/e796_beam_20O_target_2H_light_3H_Eex_0.00_nPS_0_pPS_0.root",
    //     "/media/Data/E796v2/Simulation/Outputs/Eff_study/d_t_gs_1.root",
    //     "/media/Data/E796v2/Simulation/Outputs/Eff_study/d_t_gs_2.root",
    // };
    // for(int i = 0; i < dpeaks.size(); i++)
    // {
    //     deff.Add(dpeaks[i], dfiles[i]);
    //     veff.push_back({});
    //     veff.back().Add("g0", dfiles[i]);
    // }
    // deff.Draw();
    // // Do calculation
    // std::vector<Angular::DifferentialXS> dxs;
    // std::vector<Angular::Comparator> dcomp;
    // for(int i = 0; i < dpeaks.size(); i++)
    // {
    //     dxs.push_back(Angular::DifferentialXS {&ivs, &fitter, &veff[i], &exp});
    //     dxs.back().DoFor({"g0"});
    //     // and compare
    //     dcomp.push_back(Angular::Comparator {dpeaks[i], dxs.back().Get("g0")});
    //     dcomp.back().Add("l = 2 Franck", "./Inputs/gs/Franck/gs.xs");
    //     dcomp.back().Fit(thetaCMMin, thetaCMMax);
    //     dcomp.back().Draw();
    // }


    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hCM->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();
}
