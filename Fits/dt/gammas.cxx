#include "Rtypes.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TFitResult.h"
#include "TGraphErrors.h"
#include "TString.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

#include "../../Selector/Selector.h"
void gammas()
{
    // Read fit resutls
    auto* file {new TFile {TString::Format("./Outputs/fit_%s.root", gSelector->GetFlag().c_str())}};
    auto* names {file->Get<std::vector<std::string>>("ParNames")};
    auto* res {file->Get<TFitResult>("FitResult")};

    // Fill
    auto* gg {new TGraphErrors};
    gg->SetTitle("#Gamma with E_{x} for 20O(d,t);E_{x} [MeV];#Gamma [MeV]");
    std::set<std::string> count;
    for(int i = 0; i < names->size(); i++)
    {
        auto it {names->at(i).find_first_of("_")};
        auto key {names->at(i).substr(0, it)};
        if(key.find_first_of("v") != std::string::npos)
            count.insert(key);
    }
    std::cout << "N of voigts : " << count.size() << '\n';
    for(const auto& v : count)
    {
        auto it {std::find_if(names->begin(), names->end(),
                              [&](const std::string& str) { return str.find(v) != std::string::npos; })};
        if(it != names->end())
        {
            auto idx {std::distance(names->begin(), it)};
            auto ex {res->Parameter(idx + 1)};
            auto uex {res->ParError(idx + 1)};
            auto gamma {res->Parameter(idx + 3)};
            auto ugamma {res->ParError(idx + 3)};
            gg->SetPoint(gg->GetN(), ex, gamma);
            gg->SetPointError(gg->GetN() - 1, uex, ugamma);
        }
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Gamma canvas for 20O(d,t)"}};
    gg->SetLineWidth(2);
    gg->SetMarkerStyle(24);
    gg->SetLineColor(kOrange);
    gg->SetMarkerColor(kOrange);
    gg->Draw("ap");
}
