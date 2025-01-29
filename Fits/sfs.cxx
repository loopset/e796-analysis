#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TString.h"

#include "FitInterface.h"
#include "PhysSF.h"

#include <memory>
#include <string>
#include <vector>
void sfs()
{
    // Plot sfs for these states
    std::vector<std::string> files {"./dt/"};

    // Multigraphs
    std::vector<TMultiGraph*> mgs;

    // Process
    for(const auto& path : files)
    {
        // Multigraph
        mgs.push_back(new TMultiGraph);
        mgs.back()->SetTitle(TString::Format("%s;E_{x} [MeV];SF", path.c_str()));
        // Interface
        Fitters::Interface inter;
        inter.Read(path + "Outputs/interface.root", path + "Outputs/fit_juan_RPx.root");
        // SFs
        auto sfs {std::make_unique<TFile>((path + "Outputs/sfs.root").c_str())};
        for(const auto peak : inter.GetPeaks())
        {
            auto sfcol {sfs->Get<PhysUtils::SFCollection>((peak + "_sfs").c_str())};
            auto* g {new TGraphErrors};
            g->SetLineWidth(2);
            g->SetMarkerStyle(24);
            // Get Ex
            auto ex {inter.GetParameter(peak, 1)};
            auto uex {inter.GetUnc(peak, 1)};
            for(const auto& sf : sfcol->GetSFs())
            {
                g->AddPoint(ex, sf.GetSF());
                g->SetPointError(g->GetN() - 1, uex, sf.GetUSF());
            }
            // Add to mg
            mgs.back()->Add(g);
        }
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "SF canvas"}};
    c0->DivideSquare(mgs.size());
    for(int i = 0; i < mgs.size(); i++)
    {
        c0->cd(i + 1);
        mgs[i]->Draw("ap plc pmc");
    }
}
