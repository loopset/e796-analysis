#include "ActClIMB.h"
#include "ActMultiStep.h"
#include "ActTPCData.h"
#include "ActTPCDetector.h"

#include "TCanvas.h"
#include "TH2F.h"
#include "TROOT.h"
#include "TRandom.h"
#include "TStyle.h"

std::vector<ActRoot::Voxel> GetToyData()
{
    std::vector<ActRoot::Voxel> ret;
    double widthY {2};
    double fixedy {68};
    double fixedz {128};
    double q {50};
    for(int x = 0; x < 36; x++)
    {
        ret.push_back(ActRoot::Voxel(ROOT::Math::XYZPointF(x, fixedy, fixedz), q));
    }
    double y {fixedy};
    double yminus {fixedy};
    for(int x = 36; x < 94; x++)
    {
        if(0 <= y && y <= 128)
            ret.push_back(ActRoot::Voxel(ROOT::Math::XYZPointF(x, y, fixedz), q));
        if(0 <= yminus && yminus <= 128)
            ret.push_back(ActRoot::Voxel(ROOT::Math::XYZPointF(x, yminus, fixedz), q));
        // if(y == 94)
        // {
        //     for(int ix = x; ix < 128; ix++)
        //         ret.push_back(ActRoot::Voxel({ix, y, fixedz}, q));
        // }
        y += 1;
        yminus -= 1;
    }
    return ret;
}

void FillHisto(const std::vector<ActRoot::Voxel>& voxels, TH2F* h)
{
    for(const auto& voxel : voxels)
    {
        const auto& pos {voxel.GetPosition()};
        h->Fill(pos.X(), pos.Y(), voxel.GetCharge());
    }
}

void FillClusters(const std::vector<ActCluster::Cluster>& clusters, TH2F* h)
{
    for(const auto& cluster : clusters)
    {
        for(const auto& voxel : cluster.GetVoxels())
        {
            const auto& pos {voxel.GetPosition()};
            auto binx {h->GetXaxis()->FindFixBin(pos.X())};
            auto biny {h->GetYaxis()->FindFixBin(pos.Y())};
            h->SetBinContent(binx, biny, cluster.GetClusterID() + 1);
        }
    }
}

void testMultiStep()
{
    gStyle->SetPalette(kCMYK);
    gRandom->SetSeed(0);

    // Read data
    // auto data = GetToyData();
    auto data = GetToyData();
    // ClIMB
    //  ActCluster::ClIMB climb {data};
    //  climb.Run();
    //  auto clusters = climb.GetClusters();
    //  climb.CheckMatridIsClean();
    ActRoot::TPCParameters tpc {"Actar"};
    tpc.SetREBINZ(4);
    
    auto climb {std::make_shared<ActCluster::ClIMB>()};
    climb->SetMinPoints(10);
    climb->SetTPCParameters(&tpc);
    climb->Print();
    auto clusters {climb->Run(data)};
    ActCluster::MultiStep ms {};
    ms.ReadConfigurationFile();
    ms.Print();
    ms.SetClimb(climb);
    ms.SetTPCParameters(&tpc);
    ms.SetClusters(&(clusters));
    ms.Run();
    // Print info
    for(auto& c : clusters)
    {
        c.Print();
        c.GetLine().Print();
    }

    // plotting
    auto* c1 {new TCanvas("c1", "Testing cluster")};
    auto* hPad {new TH2F("hPad", "Pad;X [pad];Y [pad]", 128, 0, 128, 128, 0, 128)};
    hPad->SetStats(false);

    auto* hPadC {(TH2F*)hPad->Clone()};
    hPadC->SetTitle("Clusters in pad");
    FillClusters(clusters, hPadC);
    FillHisto(data, hPad);

    c1->DivideSquare(2);
    c1->cd(1);
    hPad->Draw("colz");
    c1->cd(2);
    hPadC->Draw("colz");
}
