#include "ActKinematics.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"

#include "TROOT.h"
#include "TString.h"

#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngGlobals.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../FitHist.h"
#include "/media/Data/E796v2/Selector/Selector.h"

void Ang(bool isLab = false)
{
    if(isLab)
        Angular::ToggleIsLab();

    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "2H")};
    ROOT::RDF::RResultPtr<TH2D> hKin {};
    if(isLab)
        hKin = df.Histo2D(HistConfig::Kin, "fThetaLight", "EVertex");
    else
        hKin = df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex");
    auto hEx {df.Histo1D(E796Fit::Expd, "Ex")};
    // Phase space
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -2)};

    // Init intervals
    double thetaMin {isLab ? 10 : 7.};
    double thetaMax {isLab ? 22. : 16.};
    double thetaStep {1};
    Angular::Intervals ivs {thetaMin, thetaMax, E796Fit::Expd, thetaStep, 1};
    // Fill
    if(isLab)
        df.Foreach([&](float thetalab, double ex) { ivs.Fill(thetalab, ex); }, {"fThetaLight", "Ex"});
    else
        df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    phase.Foreach([&](double theta, double ex, double w) { ivs.FillPS(0, theta, ex, w); },
                  {isLab ? "theta3Lab" : "theta3CM", "Eex", "weight"});
    ivs.TreatPS(2);
    if(!isLab)
        ivs.Write("./Outputs/ivs.root");

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.Configure("./Outputs/fit_" + gSelector->GetFlag() + ".root");
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);
    fitter.DrawCounts();
    if(!isLab)
        fitter.Write("./Outputs/counts.root");

    // Interface
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");
    auto peaks {inter.GetPeaks()};

    // Efficiency
    Interpolators::Efficiency eff;
    for(const auto& peak : peaks)
        eff.Add(peak, gSelector->GetApproxSimuFile("20O", "1H", "2H", inter.GetGuess(peak)), isLab ? "effLab" : "eff");
    eff.Draw()->SaveAs("./Outputs/effs.png");

    // Set experiment info
    // Recompute normalization
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/p_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);

    // Comparators!
    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.FillComp();
    if(isLab)
    {
        inter.SetCompConfig("save", "0");
        // And convert xs to lab!
        ActPhysics::Kinematics kin {"20O(p,d)@700"};
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
    inter.GetComp("g0")->QuotientPerPoint();

    // plotting
    auto* c0 {new TCanvas {"c0", "Angular canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hKin->DrawClone("colz");
    c0->cd(2);
    hEx->DrawClone();

    gSelector->SendToWebsite(TString::Format("pd%s.root", isLab ? "_lab" : "").Data(), gROOT->GetListOfCanvases());
}
