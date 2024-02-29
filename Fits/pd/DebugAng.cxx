#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TH1.h"
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
#include <vector>

#include "../../PostAnalysis/Gates.cxx"
#include "../FitHist.h"

void DebugAng()
{
    ROOT::EnableImplicitMT();
    // Read DF
    ROOT::RDataFrame df {
        "Final_Tree", "/media/Data/E796v2/PostAnalysis/RootFiles/Legacy/tree_beam_20O_target_1H_light_2H_front.root"};

    // Cuts and labels
    std::vector<std::string> labels {"(p,d) all", "(p,d) sec 1", "(p,d) sec 2"};
    std::vector<ROOT::RDF::RNode> nodes {df, df.Filter(E796Gates::rpx1, {"fRP"}), df.Filter(E796Gates::rpx2, {"fRP"})};
    std::vector<TH1D*> hExs;
    for(int i = 0; i < nodes.size(); i++)
    {
        auto h {nodes[i].Histo1D(E796Fit::Expd, "Ex")};
        h->SetTitle(labels[i].c_str());
        hExs.push_back((TH1D*)h->Clone());
    }

    // Init intervals
    double thetaCMMin {6};
    double thetaCMMax {14};
    double thetaCMStep {1};
    std::vector<std::shared_ptr<Angular::Intervals>> ivs;
    for(int i = 0; i < nodes.size(); i++)
    {
        auto iv {std::make_shared<Angular::Intervals>(thetaCMMin, thetaCMMax, E796Fit::Expd, thetaCMStep)};
        ivs.push_back(iv);
        // Fill them
        nodes[i].Foreach([&](double thetacm, double ex) { ivs.back()->Fill(thetacm, ex); }, {"ThetaCM", "Ex"});
        // ivs.back()->Draw("Ivs for " + labels[i]);
    }

    // Fit!
    std::vector<Angular::Fitter> fitters;
    std::vector<std::string> confs {"pd", "pd_1", "pd_2"};
    for(int i = 0; i < nodes.size(); i++)
    {
        fitters.push_back(Angular::Fitter {ivs[i].get()});
        auto& fitter {fitters.back()};
        fitter.Configure(("./Outputs/fit_" + confs[i] + ".root"), {});
        fitter.Run();
        // fitter.Draw("Fitter for " + labels[i]);
        fitter.ComputeIntegrals();
        fitter.DrawCounts("Counts for " + labels[i]);
    }

    // Set how many peaks are going to be compared
    std::vector<std::string> peaks {"g0"};
    std::vector<double> peaksEx {0};
    std::vector<std::string> effsnames {"eff", "eff1", "eff2"};
    // Init efficiencies and experiment settings
    std::vector<Interpolators::Efficiency> effs;
    std::vector<PhysUtils::Experiment> exps;
    std::vector<double> nts {4.562e20, 2.4e20, 2.162e20};
    // Final results
    std::vector<Angular::DifferentialXS> xss;
    for(int i = 0; i < nodes.size(); i++)
    {
        // Init eff
        effs.push_back({});
        for(int p = 0; p < peaks.size(); p++)
            effs.back().Add(peaks[p],
                            TString::Format("/media/Data/E796v2/Simulation/Outputs/"
                                            "e796_beam_20O_target_1H_light_2H_Eex_%.2f_nPS_0_pPS_0.root",
                                            peaksEx[p])
                                .Data(),
                            effsnames[i]);

        effs.back().Draw(true, "Eff for " + labels[i]);
        // Experiments
        // WARNING: Nt must change with eff length, so correct considering LISE calculation
        exps.push_back({nts[i], 279932, 30000});
        // Run!
        xss.push_back({ivs[i].get(), &fitters[i], &effs[i], &exps[i]});
        xss.back().DoFor(peaks);
        // xss.back().Draw("XS for " + labels[i]);
    }
    // And finally compare!
    std::vector<std::vector<Angular::Comparator>> comps;
    for(int i = 0; i < nodes.size(); i++)
    {
        comps.push_back({});
        // Run for each peak
        for(const auto& peak : peaks)
        {
            comps.back().push_back({"Comp for " + peak, xss[i].Get(peak)});
            auto& comp {comps.back().back()};
            comp.Add("l = 2", "./Inputs/gs_new/21.gs");
            comp.Fit(thetaCMMin, thetaCMMax);
            comp.Draw("For " + peak + " in " + labels[i]);
        }
    }
}
