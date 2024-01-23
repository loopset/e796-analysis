#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"

#include "TCanvas.h"
#include "TH1.h"
#include "TROOT.h"
#include "TString.h"

#include <utility>
#include <vector>


void HistsVeto()
{
    ROOT::EnableImplicitMT();
    ActRoot::JoinData join {"./../../configs/merger.runs"};
    ROOT::RDataFrame d {*join.Get()};

    // VETO using only ESi0 > 0, suitable for
    // 3He and 4He reactions
    auto df {d.Filter(
        [](const ActRoot::MergerData& d)
        {
            auto condSize {d.fSilLayers.size() >= 1};
            if(condSize)
                return (d.fSilLayers.front() == "f0");
            else
                return false;
        },
        {"MergerData"})};

    // Book histograms
    int ybins {200};
    std::pair<double, double> ylims {-20, 300};
    int zbins {200};
    std::pair<double, double> zlims {-20, 300};
    auto hSP {df.Histo2D(
        {"hSP", "SP with E0 > 0;Y [mm];Z [mm]", ybins, ylims.first, ylims.second, zbins, zlims.first, zlims.second},
        "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Gate and get projections
    std::vector<int> idxs {0, 2, 3, 4, 5, 7, 8, 10};
    std::vector<ROOT::RDF::RResultPtr<TH1D>> pys {idxs.size()};
    std::vector<ROOT::RDF::RResultPtr<TH1D>> pzs {idxs.size()};
    int count {-1};
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
    auto fout {std::make_unique<TFile>("./Inputs/veto_histograms.root", "recreate")};
    fout->cd();
    for(auto& h : pys)
        h->Write();
    for(auto& h : pzs)
        h->Write();
}
