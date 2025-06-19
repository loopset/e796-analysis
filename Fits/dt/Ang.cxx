#include "ActKinematics.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"

#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngGlobals.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"
#include "../FitHist.h"

void Ang(bool isLab = false)
{
    Angular::ToggleHessErrors();

    if(isLab)
        Angular::ToggleIsLab();

    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "3H")};

    // Book histograms
    ROOT::RDF::RResultPtr<TH2D> hKin {};
    if(isLab)
        hKin = df.Histo2D(HistConfig::Kin, "fThetaLight", "EVertex");
    else
        hKin = df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex");
    auto hEx {df.Histo1D(E796Fit::Exdt, "Ex")};
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 1, 0)};
    ROOT::RDataFrame phase2 {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "3H", 0, 2, 0)};
    ROOT::RDataFrame pd {"Sel_Tree", "./Inputs/Cont/tree_20O_d_d_as_d_t.root"};

    // Init intervals
    double thetaMin {isLab ? 14 : 5.5};
    double thetaMax {isLab ? 32. : 14.};
    double thetaStep {isLab ? 4 : 1.};
    int nps {static_cast<int>(2 + 1)}; // 2 nps +  1 contamination
    Angular::Intervals ivs {thetaMin, thetaMax, E796Fit::Exdt, thetaStep, nps};
    // Fill
    if(isLab)
        df.Foreach([&](float thetalab, double ex) { ivs.Fill(thetalab, ex); }, {"fThetaLight", "Ex"});
    else
        df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    // FillPS
    phase.Foreach([&](double thetacm, double ex, double weight) { ivs.FillPS(0, thetacm, ex, weight); },
                  {"theta3CM", "Eex", "weight"});
    phase2.Foreach([&](double thetacm, double ex, double weight) { ivs.FillPS(1, thetacm, ex, weight); },
                   {"theta3CM", "Eex", "weight"});
    pd.Foreach([&](double thetaCM, double ex) { ivs.FillPS(2, thetaCM, ex, 1); }, {"ThetaCM", "Ex"});
    ivs.TreatPS(10, 0.2, {0, 1}); // disable smoothing for contamination ps
    if(!isLab)
        ivs.Write("./Outputs/ivs.root");

    // Fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetAllowFreeMean(true, {"v5", "v7"});
    fitter.SetAllowFreeSigma(true, {"g0"});
    // fitter.SetAllowFreeGamma(true, {"v1"});
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts(true);
    if(!isLab)
        fitter.Write("./Outputs/counts.root");

    // Interface
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");
    auto peaks {inter.GetPeaks()};
    // Remove contamination
    for(const auto& s : {"v8", "v9", "v10"})
        peaks.erase(std::remove(peaks.begin(), peaks.end(), s), peaks.end());

    // Efficiency
    Interpolators::Efficiency eff;
    for(const auto& peak : peaks)
        eff.Add(peak, gSelector->GetApproxSimuFile("20O", "2H", "3H", inter.GetGuess(peak)), isLab ? "effLab" : "eff");
    eff.Scale(0.95); // Calculated experimental rec efficiency from paper (underestimated)
    eff.Draw()->SaveAs("./Outputs/effs.png");

    // Set experiment info
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/d_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    if(!isLab)
    {
        xs.TrimX("v0", 12.75, false);
        xs.TrimX("v3", 7.5);
        xs.TrimX("v3", 13.5, false);
        xs.TrimX("v4", 7.5);
        xs.TrimX("v4", 13.5, false);
        xs.TrimX("v7", 7);
        // xs.TrimX("v8", 6.5);
        // xs.TrimX("v8", 11.5, false);
        xs.Write("./Outputs/");
    }

    // Comparators!
    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.FillComp();
    if(isLab)
    {
        inter.SetCompConfig("save", "0");
        // And convert xs to lab!
        ActPhysics::Kinematics kin {"20O(d,t)@700"};
        for(const auto& peak : inter.GetPeaks())
        {
            auto comp {inter.GetComp(peak)};
            auto theo {comp->GetTheoGraphs()};
            for(const auto& [name, gtheo] : theo)
            {
                auto trans {kin.TransfromCMCrossSectionToLab(gtheo)};
                comp->Replace(name, trans);
                delete trans;
            }
        }
    }
    inter.FitComp();
    if(!isLab)
        inter.WriteComp("./Outputs/sfs.root");

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hKin->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();

    gSelector->SendToWebsite(TString::Format("dt%s.root", isLab ? "_lab" : "").Data(), gROOT->GetListOfCanvases());
}
