#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TH2.h"
#include "TROOT.h"
#include "TString.h"

#include <string>

#include "../../PostAnalysis/HistConfig.h"
#include "../SilVetos/GetContourFuncs.cxx"

void DriftVelocity()
{
    // Read data
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EMerge};
    datman.SetRuns(155, 175);
    auto chain {datman.GetChain()};
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};

    // For side silicons
    std::map<int, ROOT::TThreadedObject<TH2D>> hs;
    auto hModel {HistConfig::SP.GetHistogram()};
    for(const auto& i : {1, 4, 7})
        hs.emplace(i, *hModel);
    // Process
    df.Foreach(
        [&](ActRoot::MergerData& d)
        {
            if(d.fSilLayers.size() == 1)
            {
                if(d.fSilLayers.front() == "l0")
                {
                    auto n {d.fSilNs.front()};
                    if(hs.count(n))
                        hs[n].Get()->Fill(d.fSP.X(), d.fSP.Z());
                }
            }
        },
        {"MergerData"});
    df.Count().GetValue();

    // Projections
    std::map<int, TH1D*> nx, nz;
    for(auto& [idx, h] : hs)
    {
        double w {5};
        double s {0.2};
        // X Projection
        auto namex {TString::Format("Proj X %d", idx)};
        auto x {h.Merge()->ProjectionX(namex)};
        auto* fx {FindBestFit(x, w, s)};
        nx[idx] = ScaleWithFunc(x, fx);
        // Z Projection
        auto namez {TString::Format("Proj Z %d", idx)};
        auto z {h->ProjectionY(namez)};
        auto* fz {FindBestFit(z, w, s)};
        nz[idx] = ScaleWithFunc(z, fz);
    }
    // Fit to contour
    double thresh {0.65};
    double width {15};
    FitToCountour(nx, thresh, width);
    FitToCountour(nz, thresh, width);


    // Plot
    auto* c0 {new TCanvas {"c0", "Drift per silicon"}};
    c0->DivideSquare(hs.size() * 3);
    int p {1};
    for(auto& [idx, h] : hs)
    {
        c0->cd(p);
        p++;
        h->SetTitle(TString::Format("Side %d", idx));
        h.Merge()->DrawClone("colz");
    }
    p = 4;
    for(auto& [idx, n] : nx)
    {
        c0->cd(p);
        p++;
        n->Draw();
        for(auto* o : *n->GetListOfFunctions())
            if(o)
                o->DrawClone("same");
    }
    p = 7;
    for(auto& [idx, n] : nz)
    {
        c0->cd(p);
        p++;
        n->Draw();
    }
}
