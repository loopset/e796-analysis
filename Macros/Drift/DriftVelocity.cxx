#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TROOT.h"
#include "TString.h"

#include <fstream>
#include <iostream>
#include <string>

void SiliconCountourGetter(TH1* proj)
{
    // Set parameters
    auto leftMin {70.};
    auto leftMax {85.};
    auto rightMin {90.};
    auto rightMax {110.};
    // Build functions
    auto* fleft {
        new TF1("fleft", "0.5 * [0] * (1 + (TMath::Erf((x - [1]) / (TMath::Sqrt(2) * [2]))))", leftMin, leftMax)};
    auto* fright {
        new TF1("fright", "0.5 * [0] * (1 - (TMath::Erf((x - [1]) / (TMath::Sqrt(2) * [2]))))", rightMin, rightMax)};
    // Initial parameters
    fleft->SetParameters(180, (leftMax + leftMin) / 2, 2);
    fright->SetParameters(160, (rightMax + rightMin) / 2, 2);
    // Fit
    for(auto& f : {fleft, fright})
    {
        f->SetParLimits(1, 0, 200);
        f->SetParLimits(2, 0, 4);
        proj->Fit(f, "R+");
    }
}

void DriftVelocity()
{
    // ROOT::EnableImplicitMT();

    // Get data
    ActRoot::JoinData join {"./../../configs/merger.runs"};
    // join->Print();
    ROOT::RDataFrame d {*join.Get()};
    // ROOT::RDataFrame d {"ACTAR_Merged", "./../RootFiles/Merger/Merger_Run_0156.root"};
    // d.Describe().Print();
    // gate on events stopped in first sil layer
    auto df {
        d.Filter([](const ROOT::VecOps::RVec<std::string>& layers) { return layers.size() == 1; }, {"fSilLayers"})};

    // Config histograms
    auto hLeft {df.Filter("fSilLayers.front() == \"l0\"")
                    .Histo2D({"hLeft", "Left;X [pad];Z [tb]", 200, -20, 150, 200, -20, 150}, "fSP.fCoordinates.fX",
                             "fSP.fCoordinates.fZ")};

    auto hFront {df.Filter("fSilLayers.front() == \"f0\"")
                     .Histo2D({"hFront", "Front;Y [pad];Z [tb]", 200, -20, 150, 200, -20, 150}, "fSP.fCoordinates.fY",
                              "fSP.fCoordinates.fZ")};

    auto hFront1 {df.Filter("fSilLayers.front() == \"f1\"")
                      .Histo2D({"hFront1", "Front 1;Y [pad];Z [tb]", 200, -20, 150, 200, -20, 150},
                               "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // Gate on silicon side to get proper drift velocity
    auto drift {df.Filter(
        [](const ROOT::VecOps::RVec<std::string>& silL, const ROOT::RVecF& silN)
        {
            bool isSide {silL.front() == "l0"};
            bool isN {silN.front() == 7};
            return isSide && isN;
        },
        {"fSilLayers", "fSilNs"})};
    auto hDrift {drift.Histo2D({"hDrift", "Drift histo;X [pad];Z [tb]", 200, -20, 150, 200, -20, 150},
                               "fSP.fCoordinates.fX", "fSP.fCoordinates.fZ")};
    std::cout << "hDrift entries : " << hDrift->GetEntries() << '\n';
    // Project along Z
    auto* proj {hDrift->ProjectionY("proj")};
    SiliconCountourGetter(proj);
    // // Write entries
    // ActRoot::CutsManager<int> cuts;
    // cuts.ReadCut(0, "./central_clusters.root");
    // std::ofstream streamer {"./central_clusters.dat"};
    // df.Foreach(
    //     [&](const ActRoot::MergerData& data)
    //     {
    //         if((cuts.IsInside(0, data.fSP.Y(), data.fSP.Z())))
    //             streamer << data.fRun << " " << data.fEntry << '\n';
    //     },
    //     {"MergerData"});
    // streamer.close();

    // plotting
    auto* c1 {new TCanvas("c1", "Preliminary SP")};
    c1->DivideSquare(2);
    c1->cd(1);
    hLeft->DrawClone("colz");
    c1->cd(2);
    hFront->DrawClone("colz");

    auto* c2 {new TCanvas("c2", "Drift canvas")};
    c2->DivideSquare(2);
    c2->cd(1);
    hDrift->DrawClone("colz");
    c2->cd(2);
    proj->Draw();
    // for(auto* o : *(proj->GetListOfFunctions()))
    //     if(o)
    //         o->Draw("same");
}
