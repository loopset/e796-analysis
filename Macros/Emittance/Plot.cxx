#include "ActLine.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TFile.h"
#include "TMath.h"
#include "TMathBase.h"

#include "Math/Vector3Dfwd.h"

#include <memory>
void Plot()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Emittance_Tree", "./Outputs/emittance.root"};
    // Compute angles
    auto def {df.Define("thetaXY",
                        [](ActPhysics::Line& l)
                        {
                            ROOT::Math::XYZVector p {l.GetDirection().X(), l.GetDirection().Y(), 0};
                            auto dot {p.Dot(ROOT::Math::XYZVector {1, 0, 0}) / p.R()};
                            return TMath::ACos(dot) * TMath::Sign(1, l.GetDirection().Y()) * TMath::RadToDeg();
                        },
                        {"Line"})
                  .Define("thetaXZ",
                          [](ActPhysics::Line& l)
                          {
                              ROOT::Math::XYZVector p {l.GetDirection().X(), 0, l.GetDirection().Z()};
                              auto dot {p.Dot(ROOT::Math::XYZVector {1, 0, 0}) / p.R()};
                              return TMath::ACos(dot) * TMath::Sign(1, l.GetDirection().Z()) * TMath::RadToDeg();
                          },
                          {"Line"})};

    // Define models
    ROOT::RDF::TH2DModel mEmittance {"hEmitt", "Emittance;Y [pad];Z [btb]", 500, 0, 256, 300, 0, 256};
    auto hBegin {def.Histo2D(mEmittance, "AtBegin.fCoordinates.fY", "AtBegin.fCoordinates.fZ")};
    auto hEnd {def.Histo2D(mEmittance, "AtEnd.fCoordinates.fY", "AtEnd.fCoordinates.fZ")};
    auto hPointZ {def.Histo1D("Line.fPoint.fCoordinates.fZ")};
    auto hDirZ {def.Histo1D("Line.fDirection.fCoordinates.fZ")};
    auto hThetaXY {def.Histo1D("thetaXY")};
    auto hThetaXZ {def.Histo1D("thetaXZ")};
    // Trajectories plot
    double maxxy {257};
    double maxz {320};
    int nbinsxy {200};
    int nbinsz {250};
    ROOT::TThreadedObject<TH2D> hxz {"hxz", "XZ trajectories;X [mm];Z [mm]", nbinsxy, 0, maxxy, nbinsz, 0, maxz};
    ROOT::TThreadedObject<TH2D> hyz {"hyz", "YZ trajectories;Y [mm];Z [mm]", nbinsxy, 0, maxxy, nbinsz, 0, maxz};
    def.Foreach(
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
    hThetaXY->DrawClone();
    // hPointZ->DrawClone();
    c0->cd(4);
    hThetaXZ->DrawClone();
    // hDirZ->DrawClone();
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
