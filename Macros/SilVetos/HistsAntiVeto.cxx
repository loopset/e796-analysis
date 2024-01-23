#include "ActJoinData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TROOT.h"
#include "TString.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

void HistsAntiVeto()
{
    ROOT::EnableImplicitMT();
    ActRoot::JoinData join {"./../../configs/merger.runs"};
    ROOT::RDataFrame d {*join.Get()};

    // VETO using only ESi0 > 0, suitable for
    // 3He and 4He reactions
    // Get ANTIVETO condition
    std::set<int> maskedSiN {1, 6, 9};
    auto antiveto {d.Filter(
        [&](const ROOT::VecOps::RVec<std::string>& siL, const ROOT::RVecF& siN)
        {
            // Contains energy in F0
            bool isInF0 {std::find(siL.begin(), siL.end(), "f0") != siL.end()};
            // Contains energy in F1
            bool isInF1 {std::find(siL.begin(), siL.end(), "f1") != siL.end()};
            // Impacted silicons by index
            bool isMasked {};
            // Force coincidence of index in 0 and 1
            bool shareIndex {};
            if(isInF0 && isInF1)
            {
                shareIndex = (siN[0] == siN[1]);

                for(const auto& mask : maskedSiN)
                {
                    if(siN.front() == mask)
                        isMasked = true;
                }
            }
            return isInF0 && isInF1 && shareIndex && !isMasked;
        },
        {"fSilLayers", "fSilNs"})};

    auto df = antiveto; //.Filter("fRP.fCoordinates.fX > 170");
    df = df.Define("ESil0", "fSilEs[0]").Define("ESil1", "fSilEs[1]").Define("ESum", "fSilEs[0] + fSilEs[1]");

    // Book histograms
    int ybins {250};
    std::pair<double, double> ylims {-20, 300};
    int zbins {250};
    std::pair<double, double> zlims {-20, 300};
    auto hSP {df.Histo2D({"hSP", "SP with E0 > 0 && E1 > 0;Y [mm];Z [mm]", ybins, ylims.first, ylims.second, zbins,
                          zlims.first, zlims.second},
                         "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};
    auto hPID {df.Histo2D({"hPID", "PID in gas;E_{Sil0} [MeV];Q_{ave}", 200, 0, 50, 500, 0, 2000}, "ESil0", "fQave")};

    // Get projections
    std::vector<int> idxs {0, 2, 3, 4, 5, 7, 8, 10};
    std::vector<ROOT::RDF::RResultPtr<TH1D>> pys(idxs.size());
    std::vector<ROOT::RDF::RResultPtr<TH1D>> pzs(idxs.size());
    // Filter and fill
    int count = -1;
    for(const auto& idx : idxs)
    {
        count++;
        auto cut {df.Filter(TString::Format("fSilNs.front() == %d", idx).Data())};
        std::cout << "For idx " << idx << " count : " << cut.Count().GetValue() << '\n';
        pys[count] =
            cut.Histo1D({TString::Format("py%d", idx), TString::Format("Y proj for %d;Y [mm]", idx), 250, -20, 300},
                        "fSP.fCoordinates.fY");
        pzs[count] =
            cut.Histo1D({TString::Format("pz%d", idx), TString::Format("Z proj for %d;Z [mm]", idx), 250, -20, 300},
                        "fSP.fCoordinates.fZ");
    }

    // plot
    auto* c1 {new TCanvas("c1", "Veto mode 3He and 4He")};
    hSP->DrawClone("colz");
    // cut.DrawAll();

    auto* cpy {new TCanvas("cpy", "Y projection canvas")};
    cpy->DivideSquare(pys.size());
    for(int i = 0; i < pys.size(); i++)
    {
        cpy->cd(i + 1);
        pys[i]->DrawClone();
    }

    auto* cpz {new TCanvas("cpz", "Z projection canvas")};
    cpz->DivideSquare(pzs.size());
    for(int i = 0; i < pzs.size(); i++)
    {
        cpz->cd(i + 1);
        pzs[i]->DrawClone();
    }

    // Write them
    auto fout {std::make_unique<TFile>("./Inputs/antiveto_histograms.root", "recreate")};
    fout->cd();
    for(auto& h : pys)
        h->Write();
    for(auto& h : pzs)
        h->Write();

    // auto* c2 {new TCanvas("c2", "Debug canvas")};
    // c2->DivideSquare(4);
    // c2->cd(1);
    // hPID->DrawClone("colz");
    // hQaveE0->SetMarkerStyle(6);
    // hQaveE0->SetMarkerColor(kRed);
    // hQaveE0->DrawClone("scat same");
    // c2->cd(2);
    // hQaveESil->DrawClone("colz");
    // c2->cd(3);
    // hTwoSils->DrawClone("colz");
}
