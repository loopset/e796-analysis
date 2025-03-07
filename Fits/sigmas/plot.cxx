#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TString.h"

#include "FitInterface.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../../Selector/Selector.h"
void plot()
{
    // Set files to read
    std::vector<std::string> labels {"dd", "pp", "dt", "pd"};
    std::vector<std::pair<std::string, std::string>> pairs {{"2H", "2H"}, {"1H", "1H"}, {"2H", "3H"}, {"1H", "2H"}};
    std::vector<TMultiGraph*> mgs;

    // Canvas to draw fits
    std::vector<TCanvas*> cs(2);
    for(int i = 0; i < cs.size(); i++)
    {
        auto& c {cs[i]};
        c = new TCanvas {TString::Format("c%d", i), "Sigma canvas"};
        c->DivideSquare(4);
    }

    int idx {};
    int ic {};
    int ip {};
    for(const auto& label : labels)
    {
        mgs.push_back(new TMultiGraph);
        auto& mg {mgs.back()};
        mg->SetTitle((label + ";E_{x} [MeV];#sigma [MeV]").c_str());

        // Analysis
        std::string path {"../" + label + "/"};
        auto fitfile {path + "Outputs/fit_juan_RPx.root"};
        auto interfile {path + "Outputs/interface.root"};
        Fitters::Interface inter;
        inter.Read(interfile, fitfile);

        auto* gana {new TGraphErrors};
        for(const auto& peak : inter.GetPeaks())
        {
            auto x {inter.GetParameter(peak, 1)};
            auto ux {inter.GetUnc(peak, 1)};
            auto y {inter.GetParameter(peak, 2)};
            auto uy {inter.GetUnc(peak, 2)};
            gana->AddPoint(x, y);
            gana->SetPointError(gana->GetN() - 1, ux, uy);
        }
        // Style options
        gana->SetMarkerStyle(24);
        gana->SetLineWidth(2);
        mg->Add(gana);

        // Simulation
        Interpolators::Sigmas sigmas {gSelector->GetSigmasFile(pairs[idx].first, pairs[idx].second).Data()};
        mg->Add(sigmas.GetGraph());

        // Simulation with cross-sections off
        auto legacy {TString::Format("../../Simulation/Outputs/juan_RPx/sigmas_noxs/sigmas_20O_%s_%s.root",
                                     pairs[idx].first.c_str(), pairs[idx].second.c_str())};
        auto* gleg {Interpolators::Sigmas(legacy.Data()).GetGraph()};
        gleg->SetLineStyle(2);
        mg->Add(gleg);

        // Draw!
        cs[ic]->cd(ip + 1);
        Fitters::ReadDrawGlobalFit(fitfile);
        cs[ic]->cd(ip + 2);
        mg->SetMinimum(0);
        mg->SetMaximum(0.5);
        mg->Draw("apl");

        // Update indices
        idx++;
        if(idx % 2 == 0)
            ic++;
        ip += 2;
        if(ip > 2)
            ip = 0;
    }

    // Save canvas to website!
    auto f {std::make_unique<TFile>("../../website/RootFiles/sigmas.root", "recreate")};
    for(auto c : cs)
        c->Write();
}
