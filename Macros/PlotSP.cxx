#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

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
    // ROOT::EnableImplicitMT();

    ActRoot::JoinData data {"../configs/merger.runs"};
    ROOT::RDataFrame d {*data.Get()};
    // d.Describe().Print();

    // Gate on events stopped in first layer
    auto df {d.Filter("fSilLayers.size() == 1")};

    // Book histograms
    auto hF0 {
        df.Filter("fSilLayers.front() == \"f0\"")
            .Histo2D({"hF0", "F0 SPs", 200, -20, 300, 200, -20, 300}, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};
    auto hL0 {
        df.Filter("fSilLayers.front() == \"l0\"")
            .Histo2D({"hL0", "L0 SPs", 200, -20, 276, 200, -20, 276}, "fSP.fCoordinates.fX", "fSP.fCoordinates.fZ")};
    // FOr computing ZOffset
    auto hZOff {
        df.Filter("fSilLayers.front() == \"f0\"")
            .Histo1D({"hZOff", "ZOffset", hF0->GetNbinsY(), hF0->GetYaxis()->GetXmin(), hF0->GetYaxis()->GetXmax()},
                     "fSP.fCoordinates.fZ")};

    // Write
    std::ofstream streamer {"./debug_sp_sil1.dat"};
    ActRoot::CutsManager<int> cut;
    cut.ReadCut(0, "./Cuts/debug_sp_sil1.root");
    df.Foreach(
        [&](const ActRoot::MergerData& d)
        {
            if(d.fSilNs.front() == 1 && cut.IsInside(0, d.fSP.Y(), d.fSP.Z()))
                streamer << d.fRun << " " << d.fEntry << '\n';
        },
        {"MergerData"});
    streamer.close();

    int nsils {11};
    std::vector<TH2D*> hs(nsils);
    for(int s = 0; s < nsils; s++)
        hs[s] = new TH2D(TString::Format("hSil%d", s), TString::Format("SP for sil %d;Y [mm];Z [mm]", s), 200, -20, 300,
                         200, -20, 300);

    // Fill them
    df.Foreach(
        [&](const ROOT::VecOps::RVec<std::string>& silL, const ROOT::RVecF& silN, const ROOT::Math::XYZPointF& sp)
        {
            if(silL.front() == "f0")
                hs[silN.front()]->Fill(sp.Y(), sp.Z());
        },
        {"fSilLayers", "fSilNs", "fSP"});


    // plotting
    auto* c1 {new TCanvas("c1", "Silicon points canvas")};
    c1->DivideSquare(4);
    c1->cd(1);
    hF0->DrawClone("colz");
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
                hs[idx]->Draw("colz");
            }
            idx++;
        }
    }
}
