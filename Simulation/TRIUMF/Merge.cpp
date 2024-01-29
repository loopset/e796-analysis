#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TH1.h"
#include "TMath.h"
#include "TROOT.h"
#include "TString.h"

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
    double Nt {5.56e20};         // cm-2, after multiplying by actar's length
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
    for(auto& df : dfs)
    {
        // compute scaling factor
        double scaling {(Nt * Np * xs) / Nit};
        // Get temporary histogram
        auto h {df.Histo1D(
            {"h", "Ex in for loop", hEx->GetNbinsX(), hEx->GetXaxis()->GetXmin(), hEx->GetXaxis()->GetXmax()}, "Eex")};
        h->Scale(scaling);
        hEx->Add(h.GetPtr());
    }

    // plot
    auto* c0 {new TCanvas {"c0", "Merger canvas"}};
    hEx->Draw("histe");
}
