#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TSystem.h"
#include "TVirtualPad.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"

#include <map>
#include <string>
#include <vector>

#include "../../../PostAnalysis/HistConfig.h"
#include "../../../Selector/Selector.h"
#include "../../FitHist.h"

std::map<std::string, Angular::Comparator> ang(const std::string& which)
{
    gSystem->cd("/media/Data/E796v2/Fits/dd/");
    gSelector->SetFlag("juan_RPx");
    ROOT::RDataFrame data {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "2H")};
    auto df {data.Filter(
        [&](float phi)
        {
            if(which == "low")
                return (74 <= phi) && (phi <= 80);
            else if(which == "middle")
                return (88 <= phi) && (phi <= 93);
            else if(which == "up")
                return (100 <= phi) && (phi <= 106);
            else
                return false;
        },
        {"fPhiLight"})};

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Exdd, "Ex")};
    // Phase space 19O
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, 1, 0)};
    auto hPS {phase.Histo1D(E796Fit::Exdd, "Eex", "weight")};

    // Init intervals
    double thetaMin {15};
    double thetaMax {22};
    double thetaStep {1};
    Angular::Intervals ivs {thetaMin, thetaMax, E796Fit::Exdd, thetaStep, 1};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    phase.Foreach([&](double thetacm, double ex, double w) { ivs.FillPS(0, thetacm, ex, w); },
                  {"theta3CM", "Eex", "weight"});
    ivs.TreatPS(2);
    ivs.FitPS("pol4");
    ivs.ReplacePSWithFit();
    ivs.Draw();

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetAllowFreeMean(true);
    // fitter.SetFreeMeanRange(0.1);
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitter.Run();
    fitter.Draw();
    // fitter.DrawCounts();

    // Interface
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");
    auto peaks {inter.GetKeys()};

    // Efficiency
    gSelector->SetFlag("phi_" + which);
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
    {
        const auto& peak {peaks[p]};
        eff.Add(peak, gSelector->GetSimuFile("20O", "2H", "2H", inter.GetGuess(peak)).Data(), "eff");
    }
    // Draw to check is fine
    // eff.Draw();

    // Set experiment info
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/d_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    xs.TrimX("g1", 15.75);
    xs.TrimX("g1", 20, false);
    xs.TrimX("g2", 16.2);
    xs.TrimX("g3", 17.8);

    // Init comparators!
    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.DoComp();
    // inter.GetComp("g0")->DrawSFfromIntegral(true);
    // inter.GetComp("g1")->DrawSFfromIntegral(true);
    return inter.GetComps();
}

void xs_by_phi()
{
    ROOT::EnableImplicitMT();
    gSelector->SetTarget("2H");
    gSelector->SetLight("2H");
    std::vector<std::string> flags {"low", "middle", "up"};
    std::vector<std::map<std::string, Angular::Comparator>> comps;
    for(const auto& flag : flags)
    {
        comps.push_back(ang(flag));
    }
    gSelector->SetFlag("juan_RPx");
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3)};
    auto hPhi {df.Histo1D("fPhiLight")};

    std::vector<std::string> states {"g0", "g1"};
    std::vector<TMultiGraph*> mgs;
    std::vector<Interpolators::Efficiency> effs;
    for(int i = 0; i < states.size(); i++)
    {
        mgs.push_back(new TMultiGraph);
        auto& mg {mgs.back()};
        mg->SetTitle(TString::Format("%s;#theta_{CM} [#circ];d#sigma/d#Omega [mb/sr]", states[i].c_str()));
        effs.push_back(Interpolators::Efficiency());
        for(int j = 0; j < flags.size(); j++)
        {
            auto* gexp {comps[j][states[i]].GetExp()};
            gexp->SetTitle(flags[j].c_str());
            mg->Add(gexp);
            gSelector->SetFlag("phi_" + flags[j]);
            effs.back().Add(flags[j], gSelector->GetSimuFile(0).Data(), "eff");
        }
        // Read all xs
        auto* gexp {new TGraphErrors {TString::Format("./Outputs/xs/%s_xs.dat", states[i].c_str()), "%lg %lg %lg"}};
        gexp->SetMarkerStyle(24);
        gexp->SetLineWidth(2);
        gexp->SetTitle("all");
        mg->Add(gexp);
    }

    auto* c0 {new TCanvas {"c0", "xs by phi pp"}};
    c0->DivideSquare(mgs.size() * 2);
    for(int i = 0; i < mgs.size(); i++)
    {
        c0->cd(i + 1);
        mgs[i]->Draw("ap plc pmc");
        gPad->BuildLegend();
    }
    c0->cd(3);
    effs[0].Draw(true, "Efficiencies (different colormap)", gPad);
    c0->cd(4);
    gROOT->SetSelectedPad(nullptr);
    hPhi->DrawClone();
}
