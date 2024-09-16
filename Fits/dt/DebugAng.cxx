#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TH1.h"
#include "TMultiGraph.h"
#include "TROOT.h"
#include "TString.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../PostAnalysis/Gates.cxx"
#include "../FitHist.h"
#include "../FitUtils.cxx"

void DebugAng()
{
    ROOT::EnableImplicitMT();
    // Read DF
    ROOT::RDataFrame df {
        "Final_Tree", "/media/Data/E796v2/PostAnalysis/RootFiles/Legacy/tree_beam_20O_target_2H_light_3H_front.root"};
    // Read PS
    ROOT::RDataFrame phase {"simulated_tree", "/media/Data/E796v2/RootFiles/Old/FitJuan/"
                                              "20O_and_2H_to_3H_NumN_1_NumP_0_Ex0_Date_2022_11_29_Time_16_35.root"};
    auto hPS {phase.Histo1D(E796Fit::Exdt, "Ex_cal")};
    // Init nodes
    std::vector<ROOT::RDF::RNode> nodes {df, df.Filter(E796Gates::rpx1<>, {"fRP"}), df.Filter(E796Gates::rpx2<>, {"fRP"})};
    std::vector<std::string> labels {"(d,t) all", "(d,t) sec 1", "(d,t) sec 2"};
    std::vector<TH1D*> hExs;
    for(int n = 0; n < nodes.size(); n++)
    {
        auto h {nodes[n].Histo1D(E796Fit::Exdt, "Ex")};
        hExs.push_back((TH1D*)h->Clone());
    }
    // Treat ps
    E796Fit::TreatPS(hExs.front(), hPS.GetPtr());

    // Intervals
    double thetaCMMin {8};
    double thetaCMMax {14};
    double thetaCMStep {1};
    std::vector<std::shared_ptr<Angular::Intervals>> ivs;
    for(int n = 0; n < nodes.size(); n++)
    {
        ivs.push_back(std::make_shared<Angular::Intervals>(thetaCMMin, thetaCMMax, E796Fit::Exdt, thetaCMStep));
        // Fill them
        nodes[n].Foreach([&](double cm, double ex) { ivs.back()->Fill(cm, ex); }, {"ThetaCM", "Ex"});
    }

    // Fitters
    std::vector<Angular::Fitter> fitters;
    std::vector<std::string> confs {"dt", "dt_1", "dt_2"};
    for(int n = 0; n < nodes.size(); n++)
    {
        fitters.push_back({ivs[n].get()});
        auto& fitter {fitters.back()};
        fitter.Configure(("./Outputs/fit_" + confs[n] + ".root"), {*hPS});
        fitter.Run();
        fitter.Draw("Fitter for " + labels[n]);
        fitter.ComputeIntegrals();
        fitter.DrawCounts("Counts for " + labels[n]);
    }

    // Set peaks to be compared
    std::vector<std::string> peaks {"g0", "g2"};
    std::vector<double> peaksEx {0, 3.24};
    // Read efficiencies
    std::vector<Interpolators::Efficiency> effs;
    std::vector<std::string> effnames {"eff", "eff1", "eff2"};
    // Draw in same graph
    std::unordered_map<std::string, TMultiGraph*> mgeffs;
    for(const auto& peak : peaks)
        mgeffs[peak] = new TMultiGraph;
    // Set experiments with correct normalization
    std::vector<PhysUtils::Experiment> exps;
    std::vector<double> nts {8.212e20, 4.32e20, 3.982e20};
    // Final computation
    std::vector<Angular::DifferentialXS> xss;
    for(int n = 0; n < nodes.size(); n++)
    {
        // Eff
        effs.push_back({});
        for(int p = 0; p < peaks.size(); p++)
        {
            effs.back().Add(peaks[p],
                            TString::Format("/media/Data/E796v2/Simulation/Outputs/"
                                            "e796_beam_20O_target_2H_light_3H_Eex_%.2f_nPS_0_pPS_0.root",
                                            peaksEx[p])
                                .Data(),
                            effnames[n]);
        }
        // Exp
        exps.push_back({nts[n], 279932, 30000});
        // Run!
        xss.push_back({ivs[n].get(), &fitters[n], &effs[n], &exps[n]});
        xss.back().DoFor(peaks);
    }
    // Add to mg
    for(const auto& peak : peaks)
    {
        mgeffs[peak]->SetTitle((peak + ";#theta_{CM} [#circ];#epsilon").c_str());
        for(const auto& eff : effs)
        {
            auto* g {eff.GetGraph(peak)};
            g->SetTitle(g->GetName());
            mgeffs[peak]->Add(eff.GetGraph(peak));
        }
    }
    // Plot
    auto* c0 {new TCanvas};
    c0->DivideSquare(mgeffs.size());
    int idx {};
    for(const auto& [peak, mg] : mgeffs)
    {
        auto* p {c0->cd(idx + 1)};
        mg->Draw("apl plc pmc");
        idx++;
        p->BuildLegend();
    }

    // Compare with theory
    std::vector<std::vector<Angular::Comparator>> comps;
    std::unordered_map<std::string, std::pair<std::string, std::string>> models {
        {"g0", {"l = 2", "./Inputs/gs/Franck/gs.xs"}},
        {"g2", {"l = 1", "./Inputs/g2/l_1/21.g2"}},
    };
    for(int n = 0; n < nodes.size(); n++)
    {
        comps.push_back({});
        for(const auto& peak : peaks)
        {
            comps.back().push_back({peak, xss[n].Get(peak)});
            auto& c {comps.back().back()};
            c.Add(models[peak].first, models[peak].second);
            c.Fit(thetaCMMin, thetaCMMax);
            c.Draw("Comp for " + peak + " in " + labels[n]);
        }
    }
}
