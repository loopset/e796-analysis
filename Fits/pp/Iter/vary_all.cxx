#include "ActKinematics.h"
#include "ActMergerData.h"
#include "ActParticle.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TPaveText.h"
#include "TString.h"
#include "TSystem.h"
#include "TVirtualPad.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../../../Selector/Selector.h"
#include "../../FitHist.h"

struct Return
{
    TH1D* fHist {};
    Angular::Comparator fComp {};
};

TH1D* fit(ROOT::RDF::RNode node, ROOT::RDF::RNode phase)
{
    // Ex
    auto hEx {node.Histo1D(E796Fit::Expp, "Ex")};
    // Phase
    auto hPS {phase.Histo1D(E796Fit::Expp, "Eex", "weight_trans")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr(), 1);

    // Sigmas
    Interpolators::Sigmas sigmas;
    sigmas.Read(gSelector->GetSigmasFile("1H", "1H").Data());

    auto bmax {hEx->GetMaximumBin()};
    auto gs {hEx->GetBinCenter(bmax)};
    auto offset {gs - 0};

    // Init interface
    Fitters::Interface inter;
    inter.AddState("g0", {400, 0, 0.3}, "0+");
    inter.AddState("g1", {100, 1.67, 0.3}, "2+");
    inter.AddState("g2", {50, 4.1, 0.3}, "2+");
    inter.AddState("g3", {50, 5.6, 0.3}, "3-");
    inter.AddState("g4", {50, 7.8, 0.3}, "3- and 4+");
    inter.AddState("ps0", {0.455});
    inter.EndAddingStates();
    // Workaround
    std::map<std::string, double> guesses;
    for(const auto& key : inter.GetKeys())
    {
        guesses[key] = inter.GetGuess(key);
        inter.SetInitial(key, 1, inter.GetGuess(key) + offset);
        inter.SetOffsetMeanBounds(1);
        std::cout << "Key : " << key << " initial : " << inter.GetGuess(key) << '\n';
    }

    // Read previous fit
    // inter.ReadPreviousFit("./../Outputs/fit_juan_RPx.root");
    // Eval sigma from interpolator
    inter.EvalSigma(sigmas.GetGraph());
    // And fix it!
    inter.SetFix("g1", 2, true);
    inter.SetFix("g2", 2, true);
    inter.SetFix("g3", 2, true);
    inter.SetFix("g4", 2, true);


    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS}, inter.GetCte()};

    // Fitting range
    double exmin {-15};
    double exmax {12};

    // Run!
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    ("./Outputs/fit_iter.root"), "20O(p,p) fit",
                    {{"g0", "g.s"}, {"g1", "1st ex"}, {"ps0", "20O(d,d) breakup"}}, false, false);

    // Write interface with original guesses
    for(const auto& [key, val] : guesses)
    {
        inter.SetInitial(key, 1, val);
        inter.SetOffsetMeanBounds(1);
    }
    inter.Write("./Outputs/inter_iter.root");

    // Read fitted histo
    auto file {std::make_unique<TFile>("./Outputs/fit_iter.root")};
    auto* h {file->Get<TH1D>("HistoEx")};
    auto* g {file->Get<TGraph>("GraphGlobal")};
    h->GetListOfFunctions()->Add(g);
    h->SetDirectory(nullptr);

    return h;
}

std::map<std::string, Angular::Comparator> ang(ROOT::RDF::RNode node, ROOT::RDF::RNode phase)
{
    // Init intervals
    double thetaMin {18};
    double thetaMax {24};
    double thetaStep {1};
    Angular::Intervals ivs {thetaMin, thetaMax, E796Fit::Expp, thetaStep, 1};
    node.Foreach([&](double thetacm, double ex) { ivs.Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
    phase.Foreach([&](double thetacm, double ex, double w) { ivs.FillPS(0, thetacm, ex, w); },
                  {"theta3CM", "Eex", "weight_trans"});
    ivs.TreatPS(4);

    // Init fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetManualRange(-4, 12);
    fitter.Configure("./Outputs/fit_iter.root");
    fitter.Run();
    fitter.Draw();
    fitter.DrawCounts();

    Fitters::Interface inter;
    inter.Read("./Outputs/inter_iter.root");
    std::vector<std::string> peaks {"g0", "g1"};

    // Efficiency
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
        eff.Add(peaks[p], gSelector->GetSimuFile("20O", "1H", "1H", inter.GetGuess(peaks[p])).Data(), "eff");

    PhysUtils::Experiment exp {"../../norms/p_target.dat"};
    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    xs.TrimX("g0", 18.5, true);
    xs.TrimX("g0", 24.5, false);

    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    // Fixing relative paths in comps.conf
    TString pwd {gSystem->pwd()};
    gSystem->cd("/media/Data/E796v2/Fits/pp/");
    inter.ReadCompConfig("./comps.conf");
    inter.DoComp();
    gSystem->cd(pwd);

    return inter.GetComps();
}

void vary_all()
{
    // Variational parameters
    std::vector<double> es {0.95, 1, 1.05};
    std::vector<double> angles {0.95, 1, 1.05};

    // DFs
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame data {"Sel_Tree", "../../../PostAnalysis/RootFiles/Pipe3/tree_20O_1H_1H_side_juan_RPx.root"};
    ROOT::RDataFrame phase {"SimulationTTree", "../../../Simulation/Macros/Breakup/Outputs/d_breakup_trans.root"};

    // Save results
    std::vector<std::vector<Return>> rets;
    auto* hchi {new TH2D {"hChi", "#chi^2;E scaling factor; #theta scalinf factor", 5, 0.9, 1.1, 5, 0.9, 1.1}};

    ActPhysics::Kinematics kin {"20O", "p", "p", 35 * ActPhysics::Particle("20O").GetAMU()};
    std::vector<ActPhysics::Kinematics> vkins {data.GetNSlots()};
    for(auto& k : vkins)
        k = kin;
    // Rebuild Ex and thetaCM
    for(const auto& e : es)
    {
        rets.push_back({});
        for(const auto& angle : angles)
        {
            auto node {data.RedefineSlot("Ex",
                                         [&, e, angle](unsigned int slot, const ActRoot::MergerData& d, double EVertex,
                                                       double EBeam)
                                         {
                                             vkins[slot].SetBeamEnergy(EBeam);
                                             return vkins[slot].ReconstructExcitationEnergy(
                                                 e * EVertex, angle * d.fThetaLight * TMath::DegToRad());
                                         },
                                         {"MergerData", "EVertex", "EBeam"})
                           .RedefineSlot("ThetaCM",
                                         [&, e, angle](unsigned int slot, const ActRoot::MergerData& d, double EVertex,
                                                       double EBeam)
                                         {
                                             vkins[slot].SetBeamEnergy(EBeam);
                                             return vkins[slot].ReconstructTheta3CMFromLab(
                                                        e * EVertex, angle * d.fThetaLight * TMath::DegToRad()) *
                                                    TMath::RadToDeg();
                                         },
                                         {"MergerData", "EVertex", "EBeam"})};
            // Run fit and ang
            auto h = fit(node, phase);
            auto map = ang(node, phase);
            auto& comp {map["g0"]};
            auto& fitres {comp.GetTFitRes("BG")};
            hchi->Fill(e, angle, fitres.Chi2() / fitres.Ndf());
            rets.back().push_back({.fHist = h, .fComp = comp});
        }
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "comparators"}};
    c0->Divide(es.size(), angles.size());
    int pad {1};
    for(int i = 0; i < es.size(); i++)
    {
        for(int j = 0; j < angles.size(); j++)
        {
            c0->cd(pad++);
            rets[i][j].fComp.Draw("", true, true, 3, gPad);
            auto* text {new TPaveText {0.5, 0.65, 0.85, 0.95, "nbNDC"}};
            text->AddText(TString::Format("E #upoint %.2f", es[i]));
            text->AddText(TString::Format("#theta #upoint %.2f", angles[j]));
            text->Draw();
        }
    }
    auto* c1 {new TCanvas {"c1", "fits"}};
    c1->Divide(es.size(), angles.size());
    pad = 1;
    for(int i = 0; i < es.size(); i++)
    {
        for(int j = 0; j < angles.size(); j++)
        {
            c1->cd(pad++);
            rets[i][j].fHist->Draw();
            auto* text {new TPaveText {0.5, 0.65, 0.85, 0.95, "nbNDC"}};
            text->AddText(TString::Format("E #upoint %.2f", es[i]));
            text->AddText(TString::Format("#theta #upoint %.2f", angles[j]));
            text->Draw();
        }
    }

    auto* c2 {new TCanvas {"c2", "Chi2 plot"}};
    hchi->Draw("colz");
}
