#include "ActDataManager.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/TThreadedObject.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TROOT.h"
#include "TString.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

void HistsAntiVeto()
{
    ROOT::EnableImplicitMT();
    ActRoot::DataManager datman {"./../../configs/data.conf"};
    auto chain {datman.GetJoinedData()};
    ROOT::RDataFrame d {*chain};

    // ANTI-VETO using ESi0 && ESi1 > 0, suitable for
    // p, d, t reactions
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
    std::map<int, ROOT::TThreadedObject<TH1D>> pys, pzs;
    auto* pmodel {new TH1D {"pmodel", "Model", 250, -20, 300}};
    for(const auto& i : idxs)
    {
        pys.emplace(i, *pmodel);
        pys[i].Get()->SetNameTitle(TString::Format("py%d", i), TString::Format("Y proj for %d;Y [mm]", i));
        pzs.emplace(i, *pmodel);
        pzs[i].Get()->SetNameTitle(TString::Format("pz%d", i), TString::Format("Z proj for %d;Z [mm]", i));
    }
    df.Foreach(
        [&](const ActRoot::MergerData& d)
        {
            auto n {d.fSilNs.front()};
            if(pys.count(n))
            {
                pys[n].Get()->Fill(d.fSP.Y());
                pzs[n].Get()->Fill(d.fSP.Z());
            }
        },
        {"MergerData"});

    // plot
    auto* c1 {new TCanvas("c1", "Veto mode 3He and 4He")};
    hSP->DrawClone("colz");

    auto* cpy {new TCanvas("cpy", "Y projection canvas")};
    cpy->DivideSquare(pys.size());
    int pad {0};
    for(auto& [_, p] : pys)
    {
        cpy->cd(pad + 1);
        p.Merge()->DrawClone();
        pad++;
    }

    auto* cpz {new TCanvas("cpz", "Z projection canvas")};
    cpz->DivideSquare(pzs.size());
    pad = 0;
    for(auto& [_, p] : pzs)
    {
        cpz->cd(pad + 1);
        p.Merge()->DrawClone();
        pad++;
    }
    // Write them
    auto fout {std::make_unique<TFile>("./Inputs/antiveto_histograms.root", "recreate")};
    fout->cd();
    for(auto& [_, h] : pys)
        h->Write();
    for(auto& [_, h] : pzs)
        h->Write();
}
