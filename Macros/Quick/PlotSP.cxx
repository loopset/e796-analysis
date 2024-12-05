#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TH2.h"
#include "TROOT.h"
#include "TString.h"

#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"

#include <fstream>
#include <vector>

void PlotSP()
{
    ROOT::EnableImplicitMT();

    ActRoot::DataManager data {"../../configs/data.conf"};
    auto chain {data.GetJoinedData()};
    ROOT::RDataFrame d {*chain};

    // Gate on events stopped in first layer
    auto df {d.Filter("fSilLayers.size() == 1")};

    // Gate on sides
    auto f0 {df.Filter("fSilLayers.front() == \"f0\"")};
    auto l0 {df.Filter("fSilLayers.front() == \"l0\"")};


    // Book histograms
    auto hF0 {
        f0.Histo2D({"hF0", "F0 SPs", 200, -20, 300, 200, -20, 300}, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};
    auto hL0 {
        l0.Histo2D({"hL0", "L0 SPs", 200, -20, 276, 200, -20, 276}, "fSP.fCoordinates.fX", "fSP.fCoordinates.fZ")};
    // FOr computing ZOffset
    auto hZOff {
        f0.Histo1D({"hZOff", "ZOffset", hF0->GetNbinsY(), hF0->GetYaxis()->GetXmin(), hF0->GetYaxis()->GetXmax()},
                   "fSP.fCoordinates.fZ")};

    // // Write
    // std::ofstream streamer {"./Debug/sp_scat.dat"};
    // ActRoot::CutsManager<int> cut;
    // cut.ReadCut(0, "./Cuts/debug_scat.root");
    // f0.Foreach(
    //     [&](const ActRoot::MergerData& d)
    //     {
    //         if(cut.IsInside(0, d.fSP.Y(), d.fSP.Z()))
    //             streamer << d.fRun << " " << d.fEntry << '\n';
    //     },
    //     {"MergerData"});
    // streamer.close();

    int nsils {11};
    std::map<int, ROOT::TThreadedObject<TH2D>> hs;
    auto* hmodel {new TH2D {"hmodel", "Model", 200, -20, 300, 200, -20, 300}};
    for(int s = 0; s < nsils; s++)
    {
        hs.emplace(s, *hmodel);
        hs[s].Get()->SetNameTitle(TString::Format("h%d", s), TString::Format("SP for %d;Y [mm];Z [mm]", s));
    }
    // Fill them
    df.Foreach(
        [&](const ROOT::VecOps::RVec<std::string>& silL, const ROOT::RVecF& silN, const ROOT::Math::XYZPointF& sp)
        {
            if(silL.front() == "f0")
                hs[silN.front()].Get()->Fill(sp.Y(), sp.Z());
        },
        {"fSilLayers", "fSilNs", "fSP"});


    // plotting
    auto* c1 {new TCanvas("c1", "Silicon points canvas")};
    c1->DivideSquare(4);
    c1->cd(1);
    hF0->DrawClone("colz");
    // cut.DrawAll();
    c1->cd(2);
    hL0->DrawClone("colz");
    c1->cd(3);
    hZOff->DrawClone();

    std::vector<TCanvas*> cs(3);
    int idx {0};
    for(int c = 0; c < cs.size(); c++)
    {
        cs[c] = new TCanvas(TString::Format("cSP%d", c));
        cs[c]->DivideSquare(4);
        for(int p = 0; p < 4; p++)
        {
            if(idx < hs.size())
            {
                cs[c]->cd(p + 1);
                hs[idx].Merge()->DrawClone("colz");
            }
            idx++;
        }
    }
}
