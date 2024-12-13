#include "ActLine.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TFile.h"

#include <memory>
void Plot()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Emittance_Tree", "./Outputs/emittance.root"};

    // Define models
    ROOT::RDF::TH2DModel mEmittance {"hEmitt", "Emittance;Y [pad];Z [btb]", 500, 0, 256, 300, 0, 256};
    auto hBegin {df.Histo2D(mEmittance, "AtBegin.fCoordinates.fY", "AtBegin.fCoordinates.fZ")};
    auto hEnd {df.Histo2D(mEmittance, "AtEnd.fCoordinates.fY", "AtEnd.fCoordinates.fZ")};
    auto hPointZ {df.Histo1D("Line.fPoint.fCoordinates.fZ")};
    auto hDirZ {df.Histo1D("Line.fDirection.fCoordinates.fZ")};
    // Trajectories plot
    double maxxy {257};
    double maxz {320};
    int nbinsxy {200};
    int nbinsz {250};
    ROOT::TThreadedObject<TH2D> hxz {"hxz", "XZ trajectories;X [mm];Z [mm]", nbinsxy, 0, maxxy, nbinsz, 0, maxz};
    ROOT::TThreadedObject<TH2D> hyz {"hyz", "YZ trajectories;Y [mm];Z [mm]", nbinsxy, 0, maxxy, nbinsz, 0, maxz};
    df.Foreach(
        [&](ActPhysics::Line& line)
        {
            for(int b = 1; b <= nbinsxy; b++)
            {
                auto x {hxz.Get()->GetXaxis()->GetBinCenter(b)};
                auto pos {line.MoveToX(x)};
                hxz.Get()->Fill(pos.X(), pos.Z());
                hyz.Get()->Fill(pos.Y(), pos.Z());
            }
        },
        {"Line"});

    // Print statistics
    std::cout << "-> Beginning : " << '\n';
    std::cout << "   sigmaY : " << hBegin->GetStdDev(1) * 2.35 << '\n';
    std::cout << "   sigmaZ : " << hBegin->GetStdDev(2) * 2.35 << '\n';
    std::cout << "-> End       : " << '\n';
    std::cout << "   sigmaY : " << hEnd->GetStdDev(1) * 2.35 << '\n';
    std::cout << "   sigmaZ : " << hEnd->GetStdDev(2) * 2.35 << '\n';


    // Plot
    auto* c0 {new TCanvas {"c0", "Emittance canvas"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hBegin->DrawClone("colz");
    c0->cd(2);
    hEnd->DrawClone("colz");
    c0->cd(3);
    hPointZ->DrawClone();
    c0->cd(4);
    hDirZ->DrawClone();
    c0->cd(5);
    hxz.Merge()->DrawClone("colz");
    c0->cd(6);
    hyz.Merge()->DrawClone("colz");

    // Save objects to file
    auto file {std::make_unique<TFile>("./Outputs/histos.root", "recreate")};
    hBegin->Write("hBegin");
    hEnd->Write("hEnd");
    hxz->Write("hXZ");
    hyz->Write("hYZ");
}
