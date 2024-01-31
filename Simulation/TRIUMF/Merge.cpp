#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"
#include "Rtypes.h"

#include "TAttLine.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TLegend.h"
#include "TMath.h"
#include "TLatex.h"
#include "TROOT.h"
#include "TString.h"
#include "TStyle.h"
#include "TVirtualPad.h"

#include <string>
#include <vector>
void Merge()
{
    ROOT::EnableImplicitMT();

    std::vector<std::string> files {"./Outputs/transfer_TRIUMF_Eex_0.000_nPS_0_pPS_0.root",
                                    "./Outputs/transfer_TRIUMF_Eex_0.130_nPS_0_pPS_0.root",
                                    "./Outputs/transfer_TRIUMF_Eex_0.435_nPS_0_pPS_0.root"};

    // Read dfs
    std::vector<ROOT::RDF::RNode> dfs;
    for(const auto& file : files)
    {
        dfs.push_back(ROOT::RDataFrame {"SimulationTTree", file});
    }
    // Compute scaling factors
    double Nt {5.56e20};              // cm-2, after multiplying by actar's length
    double Np {3000 * 6 * 24 * 3600}; // 3000 pps x 6 days
    double Nit {1.e6};
    double xs {4 * TMath::Pi() *
               1e-27}; // not computed here, should be obtained from XSSampler as the absolute xs -> ~ 1 mbarn

    // Set histogram
    int nbins {200};
    double xmin {-2};
    double xmax {4};
    auto* hEx {new TH1D {
        "hEx", TString::Format("Ex for all peaks;E_{x} [MeV];Counts / %.0f keV", (xmax - xmin) / nbins * 1000), nbins,
        xmin, xmax}};
    hEx->Sumw2();
    std::vector<TH1D*> hs;
    for(auto& df : dfs)
    {
        // compute scaling factor
        double scaling {(Nt * Np * xs) / Nit};
        // Get temporary histogram
        auto h {df.Histo1D(
            {"h", "Ex in for loop", hEx->GetNbinsX(), hEx->GetXaxis()->GetXmin(), hEx->GetXaxis()->GetXmax()}, "Eex")};
        h->Scale(scaling);
        hEx->Add(h.GetPtr());
        hs.push_back((TH1D*)h->Clone());
    }

    // plot
    auto* c0 {new TCanvas {"c0", "Merger canvas"}};
    gStyle->SetOptStat(0);
    hEx->SetLineWidth(2);
    hEx->Draw("histe");
    std::vector<int> colors {6, 8, 46};
    std::vector<std::string> labels {"0", "0.130", "0.435"};
    auto* leg {new TLegend {0.2, 0.2}};
    leg->SetHeader("E_{x} [MeV]");
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    for(int i = 0; i < hs.size(); i++)
    {
        hs[i]->SetLineColor(colors[i]);
        hs[i]->SetLineStyle(kDashed);
        hs[i]->SetLineWidth(2);
        leg->AddEntry(hs[i], labels[i].c_str());
        hs[i]->Draw("hist same");
    }
    leg->Draw();

    // add text
    auto* latex {new TLatex{0.5, 0.5, "#font[42]{#sigma #approx 70 keV}"}};
    latex->Draw();
}
