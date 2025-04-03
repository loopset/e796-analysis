
#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngFitter.h"
#include "AngIntervals.h"
#include "FitInterface.h"
#include "Interpolators.h"
#include "PhysExperiment.h"
#include "uncertainties.hpp"

#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "../../../PostAnalysis/HistConfig.h"
#include "../../../Selector/Selector.h"

struct GraphPair
{
    TGraphErrors* fgs {};
    TGraphErrors* ffirst {};
    unc::udouble fsf {};
};

void psamp()
{
    // Read intervals
    Angular::Intervals ivs;
    ivs.Read("../Outputs/ivs.root");
    ivs.Draw();

    // Read interface
    Fitters::Interface inter;
    inter.Read("../Outputs/interface.root");
    auto peaks {inter.GetKeys()};

    // Read previous interval fits
    Angular::Fitter fitter;
    fitter.Read("../Outputs/fitter.root");
    // Ps0 index is last one
    auto idx {fitter.GetParNames().size() - 1};
    std::vector<double> psamps;
    for(const auto& res : fitter.GetTFitResults())
        psamps.push_back(res.Parameter(idx));

    // Efficiency
    Interpolators::Efficiency eff;
    for(int p = 0; p < peaks.size(); p++)
        eff.Add(peaks[p], gSelector->GetSimuFile("20O", "1H", "1H", inter.GetGuess(peaks[p])).Data(), "eff");
    eff.Draw(true);

    // Experimental settings
    gSelector->RecomputeNormalization();
    PhysUtils::Experiment exp {"../../norms/p_target.dat"};

    // And save results per condition!
    std::vector<double> factors {0, 0.25, 0.5, 0.75, 1};
    std::vector<GraphPair> res;
    auto* mg0 {new TMultiGraph};
    mg0->SetTitle("gs;#theta_{CM} [#circ];d#sigma/d#Omega [mb/sr]");
    auto* mg1 {new TMultiGraph};
    mg1->SetTitle("2^{+}_{1};#theta_{CM} [#circ];d#sigma/d#Omega [mb/sr]");
    auto* gsf {new TGraphErrors};
    gsf->SetTitle("SF with factor;Factor;SF");
    for(const auto& factor : factors)
    {
        std::cout << "-> Factor : " << factor << '\n';
        // Iteration ps0 amplitudes
        std::vector<double> itamps;
        std::transform(psamps.begin(), psamps.end(), std::back_inserter(itamps), [&](double e) { return e * factor; });

        // Fitter
        Angular::Fitter itfitter {&ivs};
        itfitter.SetFixAmpPS(0, itamps);
        itfitter.Configure(TString::Format("../Outputs/fit_%s.root", gSelector->GetFlag().c_str()).Data());
        itfitter.Run();
        // Cross-section
        Angular::DifferentialXS itxs {&ivs, &itfitter, &eff, &exp};
        itxs.DoFor(peaks);
        itxs.TrimX("g0", 18.5, true);
        itxs.TrimX("g0", 24.5, false);
        auto* gs {itxs.Get("g0")};
        auto* first {itxs.Get("g1")};
        // Comparator
        Angular::Comparator comp {"2+1", first};
        comp.Add("BG", "../Inputs/g1_BG/fort.202");
        comp.Fit();
        unc::udouble sf {comp.GetSF("BG"), comp.GetuSF("BG")};

        // Name
        auto name {TString::Format("factor = %.2f", factor)};
        for(auto* g : {gs, first})
        {
            g->SetTitle(name);
            g->SetMarkerStyle(24);
            g->SetLineWidth(2);
        }
        mg0->Add(gs);
        mg1->Add(first);
        gsf->AddPoint(factor, sf.n());
        gsf->SetPointError(gsf->GetN() - 1, 0, sf.s());

        res.push_back({.fgs = gs, .ffirst = first, .fsf = sf});
    }

    // Save to file!
    auto file {std::make_unique<TFile>("./Outputs/psamp.root", "recreate")};
    file->WriteObject(&factors, "Factors");
    mg1->Write("mgFirst");

    // Draw
    auto* c0 {new TCanvas {"c0", "psamp search"}};
    c0->DivideSquare(4);
    c0->cd(1);
    mg0->Draw("ap pmc plc");
    c0->cd(2);
    mg1->Draw("ap pmc plc");
    gPad->BuildLegend();
    c0->cd(3);
    gsf->SetLineWidth(2);
    gsf->SetMarkerStyle(25);
    gsf->Draw("apl");
}
