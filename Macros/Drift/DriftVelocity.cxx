#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActSilMatrix.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TH2.h"
#include "TMath.h"
#include "TROOT.h"
#include "TString.h"

#include <iostream>
#include <string>

#include "../../PostAnalysis/HistConfig.h"
#include "../SilVetos/GetContourFuncs.cxx"

void DriftVelocity()
{
    // Read data
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EMerge};
    // datman.SetRuns(155, 275);
    auto chain {datman.GetChain()};
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};

    // For side silicons
    std::map<int, ROOT::TThreadedObject<TH2D>> hs;
    auto hModel {HistConfig::SP.GetHistogram()};
    ROOT::TThreadedObject<TH2D> hSP {*hModel};
    for(const auto& i : {1, 2, 4, 5, 7})
        hs.emplace(i, *hModel);
    // Process
    df.ForeachSlot(
        [&](unsigned int slot, ActRoot::MergerData& d)
        {
            if(d.fSilLayers.size() == 1)
            {
                if(d.fSilLayers.front() == "l0")
                {
                    hSP.GetAtSlot(slot)->Fill(d.fSP.X(), d.fSP.Z());
                    auto n {d.fSilNs.front()};
                    if(hs.count(n))
                        hs[n].GetAtSlot(slot)->Fill(d.fSP.X(), d.fSP.Z());
                }
            }
        },
        {"MergerData"});
    df.Count().GetValue();

    // Merge
    for(auto& [idx, h] : hs)
        h.Merge();
    // After merging, slot0 contains the merged histogram also

    // Projections
    std::map<int, TH1D*> nx, nz;
    for(auto& [idx, h] : hs)
    {
        double w {5};
        double s {0.5};
        // X Projection
        auto namex {TString::Format("Proj X %d", idx)};
        auto x {h.GetAtSlot(0)->ProjectionX(namex)};
        auto* fx {FindBestFit(x, w, s)};
        nx[idx] = ScaleWithFunc(x, fx);
        // Z Projection
        auto namez {TString::Format("Proj Z %d", idx)};
        auto z {h.GetAtSlot(0)->ProjectionY(namez)};
        auto* fz {FindBestFit(z, w, s)};
        nz[idx] = ScaleWithFunc(z, fz);
    }
    // Fit to contour
    double thresh {0.65};
    double width {15};
    auto mapx = FitToCountour(nx, thresh, width);
    auto mapz = FitToCountour(nz, thresh, width);

    // Build sm
    auto* sm {new ActPhysics::SilMatrix};
    sm->SetName("Raw l0");
    auto* gh {new TGraphErrors};
    gh->SetTitle("Drift factor;Sil index;Drift factor [btb/mm]");
    gh->SetMarkerStyle(24);
    for(const auto& [idx, pair] : mapx)
    {
        sm->AddSil(idx, pair, mapz[idx]);
        gh->AddPoint(idx, 50. / sm->GetHeight(idx));
    }
    // Get mean and median
    auto mean {TMath::Mean(gh->GetN(), gh->GetY())};
    auto median {TMath::Median(gh->GetN(), gh->GetY())};
    std::cout << "-> Drift velocity from raw SPs : " << '\n';
    std::cout << "   Mean   : " << mean << " btb / mm" << '\n';
    std::cout << "   Median : " << median << " btb / mm" << '\n';


    // Plot
    auto* c0 {new TCanvas {"c0", "Drift per silicon"}};
    c0->DivideSquare(hs.size() * 3);
    int p {1};
    for(auto& [idx, h] : hs)
    {
        c0->cd(p);
        p++;
        h.GetAtSlot(0)->SetTitle(TString::Format("Side %d", idx));
        h.GetAtSlot(0)->DrawClone("colz");
    }
    p = hs.size() + 1;
    for(auto& [idx, n] : nx)
    {
        c0->cd(p);
        p++;
        n->Draw();
        for(auto* o : *n->GetListOfFunctions())
            if(o)
                o->DrawClone("same");
    }
    p = 2 * hs.size() + 1;
    for(auto& [idx, n] : nz)
    {
        c0->cd(p);
        p++;
        n->Draw();
    }

    auto* c1 {new TCanvas {"c1", "SM canvas"}};
    c1->DivideSquare(4);
    c1->cd(1);
    sm->Draw(false);
    c1->cd(2);
    gh->Draw("ap");
    c1->cd(3);
    hSP.Merge()->DrawClone("colz");
}
