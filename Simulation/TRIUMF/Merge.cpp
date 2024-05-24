#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"
#include "Rtypes.h"

#include "TAttLine.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TMath.h"
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

    // You can process all files at once:
    // by passing the vector of files to the constructor of a RDataFrame
    // but then you cannot identify by colors each peak ! (probably not that interesting)
    // ROOT::RDataFrame df {"SimulationTTree", files};

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
    std::vector<std::vector<TH1D*>> hthetas;
    for(auto& df : dfs)
    {
        hthetas.push_back({});       // Vector of vectors: we initialized an empty row for the first df
        auto& hrow {hthetas.back()}; // access the last element of a vector with .back() (a reference)
        // To gate in angle, remember the column name of the theta lab: theta3Lab
        // You can use the Filter operation passing
        // a string with the gate you want:
        // auto gated {df.Filter("60 <= theta3Lab && 70 <= theta3Lab")};
        // If you want to pass several filter automatically:
        // 1-> Create an outter vector to store the results; here hthetas;
        for(double theta = 0; theta <= 180; theta += 20)
        {
            // Create a string with the current cut: in C++ in not that easy to format a string as in python
            // ROOT helps with:
            // Boundaries of interval
            auto low {theta};
            auto up {theta + 20};
            auto str {TString::Format("%f <= theta3Lab && theta3Lab < %f", low, up)};
            // %f indicates that you are formating a float
            // Create a node: a copy of the RDF with custom cuts, new columns, etc
            auto node {df.Filter(str.Data())}; // this filters all the columns,
            // storing in the variable named node only the entries that fullfil the condition
            // You have to use the .Data() method of the TString str bc the Filter method does not allow
            // a TString argument. .Data() converts to a old-C char* type (cousas técnicas, con saber que a próxima vez
            // telo que facer así xa chega :)
            // And now get your histogram as usual (ofc using the node variable)
            auto hInner {node.Histo1D(
                {"h", str, hEx->GetNbinsX(), hEx->GetXaxis()->GetXmin(), hEx->GetXaxis()->GetXmax()}, "Eex")};
            // and push back (best idea is to store in the vector the clone of the temp histogram hInner)
            hrow.push_back((TH1D*)hInner->Clone());
            // Probably you would need to scale it before pushing back
        }

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
    // gStyle->SetOptStat(0);
    hEx->SetStats(false);
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
    auto* latex {new TLatex {0.5, 0.5, "#font[42]{#sigma #approx 70 keV}"}};
    latex->Draw();

    // If you wanted to plot all the histograms by interval
    // We will create a TCanvas for each file
    for(int f = 0; f < files.size(); f++)
    {
        // Crate a tcanvas
        // Each TCanvas must have the first argument UNIQUE, otherwise when init the new one the older one will be
        // deleted
        auto* c {new TCanvas {TString::Format("c%d", f + 1), ("Canvas for " + labels[f]).c_str()}};
        c->DivideSquare(hthetas[f].size());
        for(int i = 0; i < hthetas[f].size(); i++)
        {
            c->cd(i + 1); // +1 bc pads range from [1, max] and the i index [0, max - 1]
            // Setting titles for the axis
            hthetas[f][i]->GetXaxis()->SetTitle("E_{x} [MeV]");
            hthetas[f][i]->GetYaxis()->SetTitle("Counts");
            hthetas[f][i]->Draw();
        }
    }
}
