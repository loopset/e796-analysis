#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TPaveStats.h"
#include "TString.h"
#include "TSystem.h"
#include "TVirtualPad.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../../../Selector/Selector.h"


Angular::Fitter fitter(Angular::Intervals& ivs, std::function<void(Angular::Fitter&)>& lambdafit)
{
    // Fitter
    Angular::Fitter fitter {&ivs};
    fitter.SetAllowFreeSigma(true, {"g0"});
    // Apply lambda funtion
    lambdafit(fitter);
    fitter.Configure("./Outputs/fit_juan_RPx.root");
    fitter.Run();
    fitter.Draw();
    fitter.ComputeIntegrals(2);

    return fitter;
}

std::map<std::string, Angular::Comparator> angular(Angular::Intervals& ivs, Angular::Fitter& fitter,
                                                   std::function<void(Angular::DifferentialXS&)>& lambdaxs,
                                                   Interpolators::Efficiency& eff, PhysUtils::Experiment& exp)
{
    // Interface
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");
    std::vector<std::string> peaks {"v5", "v6", "v7"};

    // And compute differential xs!
    Angular::DifferentialXS xs {&ivs, &fitter, &eff, &exp};
    xs.DoFor(peaks);
    // Apply lambda function
    lambdaxs(xs);

    // Comparators!
    for(const auto& peak : peaks)
        inter.AddAngularDistribution(peak, xs.Get(peak));
    inter.ReadCompConfig("./comps.conf");
    inter.FillComp();
    inter.FitComp();

    return inter.GetComps();
}

void iter_v567()
{
    std::vector<std::function<void(Angular::Fitter&)>> lambdafits {
        [](Angular::Fitter& fitter) {},
        [](Angular::Fitter& fitter) { fitter.SetAllowFreeMean(true, {"v5"}); },
        [](Angular::Fitter& fitter) { fitter.SetAllowFreeMean(true, {"v5", "v6"}); },
        [](Angular::Fitter& fitter) { fitter.SetAllowFreeMean(true, {"v5", "v6", "v7"}); },
    };

    std::vector<std::function<void(Angular::DifferentialXS&)>> lambdaxs {
        [](Angular::DifferentialXS& xs) {},
        [](Angular::DifferentialXS& xs)
        {
            for(const auto& peak : {"v5", "v6", "v7"})
                xs.TrimX(peak, 7);
        },
        [](Angular::DifferentialXS& xs)
        {
            for(const auto& peak : {"v5", "v6", "v7"})
                xs.TrimX(peak, 8);
        },
        [](Angular::DifferentialXS& xs)
        {
            for(const auto& peak : {"v5", "v6", "v7"})
                xs.TrimX(peak, 12.5, false);
        },
    };

    // Get current dir
    TString pwd {gSystem->pwd()};
    gSystem->cd("/media/Data/E796v2/Fits/dt/");

    // Read ivs
    Angular::Intervals ivs;
    ivs.Read("./Outputs/rebin/ivs.root");

    // Interface
    Fitters::Interface inter;
    inter.Read("./Outputs/interface.root");

    // Efficiency
    Interpolators::Efficiency eff;
    std::vector<std::string> peaks {"v5", "v6", "v7"};
    for(const auto& peak : peaks)
        eff.Add(peak, gSelector->GetApproxSimuFile("20O", "2H", "3H", inter.GetGuess(peak)), "eff");
    eff.Scale(0.95); // reconstruction efficiency from paper

    // Experiment
    PhysUtils::Experiment exp {"../norms/d_target.dat"};

    // Init histograms
    std::vector<std::string> models {"l = 0", "l = 1", "l = 2"};
    std::vector<std::map<std::string, TH1D*>> hs;
    std::vector<std::vector<std::map<std::string, Angular::Comparator>>> comps;
    for(const auto& peak : peaks)
    {
        hs.push_back({});
        int idx {0};
        for(const auto& model : models)
        {
            auto* h {new TH1D {TString::Format("hSF%s%d", peak.c_str(), idx),
                               TString::Format("%s %s;%s SF;", peak.c_str(), model.c_str(), peak.c_str()), 200, 0, 2}};
            hs.back()[model] = h;
            idx++;
        }
    }


    // Run!
    for(auto& lfit : lambdafits)
    {
        comps.push_back({});
        // Run fitter
        auto fit {fitter(ivs, lfit)};
        // Run angular distribution
        for(auto& lxs : lambdaxs)
        {
            auto map {angular(ivs, fit, lxs, eff, exp)};
            comps.back().push_back({});
            // Get information for each state
            int idx {};
            for(const auto& peak : peaks)
            {
                auto& comp {map[peak]};
                comps.back().back()[peak] = comp;
                // And for each model
                for(const auto& model : models)
                {
                    auto sf {comp.GetSF(model)};
                    hs[idx][model]->Fill(sf);
                }
                idx++;
            }
        }
    }

    gSystem->cd(pwd);
    // Write to disk
    auto fout {std::make_unique<TFile>("../../../Publications/dt/Inputs/iter_v567.root", "recreate")};
    for(const auto& map : hs)
    {
        for(const auto& [_, h] : map)
            h->Write();
    }
    fout->Close();

    std::vector<int> colors {46, 8, 9};
    // Draw
    auto* c0 {new TCanvas {"c0", "SF canvas"}};
    c0->DivideSquare(4);
    for(int i = 0; i < hs.size(); i++)
    {
        c0->cd(i + 1);
        int j {};
        for(const auto& [key, h] : hs[i])
        {
            h->SetLineColor(colors[j]);
            h->SetLineWidth(2);
            h->Draw(j != 0 ? "sames" : "");
            gPad->Update(); // force generation of TPaveStats
            auto* pave {(TPaveStats*)h->GetListOfFunctions()->FindObject("stats")};
            pave->SetTextColor(colors[j]);
            auto offset {0.25};
            auto width {0.15};
            pave->SetX1NDC(offset + width * j);
            pave->SetX2NDC(offset + width * (j + 1));
            gPad->Update();
            j++;
        }
        gPad->BuildLegend();
    }

    for(const auto& peak : peaks)
    {
        auto* c {new TCanvas {TString::Format("c%s", peak.c_str()), TString::Format("%s canvas", peak.c_str())}};
        c->Divide(lambdafits.size(), lambdaxs.size());
        int pad {1};
        for(int i = 0; i < lambdafits.size(); i++)
        {
            for(int j = 0; j < lambdaxs.size(); j++)
            {
                c->cd(pad++);
                comps[i][j][peak].Draw("", false, true, 3, gPad, true);
            }
        }
    }
}
