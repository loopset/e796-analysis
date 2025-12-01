#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"
#include "TVirtualPad.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../../../PostAnalysis/HistConfig.h"
#include "../../../Selector/Selector.h"
#include "../../FitHist.h"

std::map<std::string, Angular::Comparator> ang(std::string which)
{
    auto it {which.find_first_of("_")};
    bool isAngle {false};
    if(it != std::string::npos)
    {
        which = std::string(which.substr(0, it));
        isAngle = true;
        std::cout << "Is angle!" << '\n';
    }
    gSystem->cd("/media/Data/E796v2/Fits/pp/");
    // ROOT::EnableImplicitMT();
    gSelector->SetFlag("juan_RPx");
    gSelector->SetTag("");
    ROOT::RDataFrame data {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "1H")};
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
    // Phase space deuton breakup
    ROOT::RDataFrame phase {"SimulationTTree",
                            "../../Simulation/Macros/Breakup/Outputs/d_breakup_trans.root"}; // set weight_trans

    // Book histograms
    auto hCM {df.Histo2D(HistConfig::KinCM, "ThetaCM", "EVertex")};
    auto hEx {df.Histo1D(E796Fit::Expp, "Ex")};

    // Init intervals
    double thetaMin {18};
    double thetaMax {24};
    double thetaStep {1};
    Angular::Intervals ivs {thetaMin, thetaMax, E796Fit::Expp, thetaStep, 1};
    // Fill
    df.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    phase.Foreach([&](double thetacm, double ex, double w) { ivs.FillPS(0, thetacm, ex, w); },
                  {"theta3CM", "Eex", "weight_trans"});
    ivs.TreatPS(4);
    ivs.Draw();

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetManualRange(-4, 12);
    fitter.Configure(TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
    fitter.Run();
    fitter.Draw();

    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");
    std::vector<std::string> peaks {"g0", "g1"};

    // Efficiency
    gSelector->SetFlag("phi_" + which);
    if(isAngle)
        gSelector->SetTag("angle_8");
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
        eff.Add(peaks[p], gSelector->GetSimuFile("20O", "1H", "1H", inter.GetGuess(peaks[p])).Data());

    // Recompute normalzation
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../norms/p_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    xs.TrimX("g0", 18.5, true);
    xs.TrimX("g0", 24.5, false);

    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.DoComp();
    inter.GetComp("g0")->ScaleToExp("BG", &exp, fitter.GetIgCountsGraph("g0"), eff.GetTEfficiency("g0"));


    return inter.GetComps();
}

void xs_by_phi()
{
    gSelector->SetTarget("1H");
    gSelector->SetLight("1H");
    std::vector<std::string> flags {"low", "middle", "up", "low_angle"};
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
            if(TString tstr {flags[j]}; tstr.Contains("angle"))
            {
                gSelector->SetFlag("phi_low");
                gSelector->SetTag("angle_8");
            }
            else
            {
                gSelector->SetFlag("phi_" + flags[j]);
                gSelector->SetTag("");
            }
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
