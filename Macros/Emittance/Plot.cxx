#include "ActLine.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/TThreadedObject.hxx"

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TMath.h"
#include "TMathBase.h"
#include "TPaveText.h"
#include "TString.h"

#include "Math/Vector3Dfwd.h"

#include <iostream>
#include <memory>

#include "../../Selector/Selector.h"
void Plot()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Emittance_Tree", "./Outputs/emittance.root"};
    // Compute angles
    auto def {df.Define("thetaXY",
                        [](ActRoot::Line& l)
                        {
                            ROOT::Math::XYZVector p {l.GetDirection().X(), l.GetDirection().Y(), 0};
                            p = p.Unit();
                            auto dot {p.Dot(ROOT::Math::XYZVector {1, 0, 0})};
                            return TMath::ACos(dot) * TMath::Sign(1, l.GetDirection().Y()) * TMath::RadToDeg();
                        },
                        {"Line"})
                  .Define("thetaXZ",
                          [](ActRoot::Line& l)
                          {
                              ROOT::Math::XYZVector p {l.GetDirection().X(), 0, l.GetDirection().Z()};
                              p = p.Unit();
                              auto dot {p.Dot(ROOT::Math::XYZVector {1, 0, 0})};
                              return TMath::ACos(dot) * TMath::Sign(1, l.GetDirection().Z()) * TMath::RadToDeg();
                          },
                          {"Line"})};

    // Define models
    ROOT::RDF::TH2DModel mEmittance {"hEmitt", "Emittance;Y [pad];Z [btb]", 500, 0, 256, 300, 0, 256};
    auto hBegin {def.Histo2D(mEmittance, "AtBegin.fCoordinates.fY", "AtBegin.fCoordinates.fZ")};
    auto hEnd {def.Histo2D(mEmittance, "AtEnd.fCoordinates.fY", "AtEnd.fCoordinates.fZ")};
    auto hPointZ {def.Histo1D("Line.fPoint.fCoordinates.fZ")};
    auto hDirZ {def.Histo1D("Line.fDirection.fCoordinates.fZ")};
    auto hBeginZ {def.Histo1D("AtBegin.fCoordinates.fZ")};
    auto hThetaXY {def.Histo1D("thetaXY")};
    auto hThetaXZ {def.Histo1D("thetaXZ")};
    auto hYthetaXY {def.Histo2D({"hYthetaXY", "Y vs #theta_{XY};Y [mm];#theta_{XY} [#circ]", 160, 80, 160, 150, -5, 5},
                                "AtBegin.fCoordinates.fY", "thetaXY")};
    auto hYthetaXZ {def.Histo2D({"hYthetaXZ", "Y vs #theta_{XZ};Y [mm];#theta_{XZ} [#circ]", 160, 80, 160, 150, -5, 5},
                                "AtBegin.fCoordinates.fY", "thetaXZ")};
    auto hZthetaXY {def.Histo2D({"hZthetaXY", "Z vs #theta_{XY};Z [mm];#theta_{XY} [#circ]", 200, 120, 220, 150, -5, 5},
                                "AtBegin.fCoordinates.fZ", "thetaXY")};
    auto hZthetaXZ {def.Histo2D({"hZthetaXZ", "Z vs #theta_{XZ};Z [mm];#theta_{XZ} [#circ]", 200, 120, 220, 150, -5, 5},
                                "AtBegin.fCoordinates.fZ", "thetaXZ")};
    auto h3d {def.Histo3D({"h3D", "Emittance histogram;Y [mm];#theta_{XY} [#circ];#theta_{XZ} [#circ]", 160, 80, 160,
                           150, -5, 5, 150, -5, 5},
                          "AtBegin.fCoordinates.fY", "thetaXY", "thetaXZ")};
    auto hprof3d {def.Profile2D(
        {"hprof3D", "Emittance histogram;Y [mm];#theta_{XY} [#circ];#theta_{XZ} [#circ]", 160, 80, 160, 150, -5, 5},
        "AtBegin.fCoordinates.fY", "thetaXY", "thetaXZ")};
    auto hDebugY {def.Histo2D({"hDebugY", "Debug Y;Y fit [mm];Y at X = 0 [mm]", 160, 80, 160, 160, 80, 160},
                              "Line.fPoint.fCoordinates.fY", "AtBegin.fCoordinates.fY")};
    // Trajectories plot
    double maxxy {257};
    double maxz {320};
    int nbinsxy {200};
    int nbinsz {250};
    ROOT::TThreadedObject<TH2D> hxz {"hxz", "XZ trajectories;X [mm];Z [mm]", nbinsxy, 0, maxxy, nbinsz, 0, maxz};
    ROOT::TThreadedObject<TH2D> hyz {"hyz", "YZ trajectories;Y [mm];Z [mm]", nbinsxy, 0, maxxy, nbinsz, 0, maxz};
    def.Foreach(
        [&](ActRoot::Line& line)
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
    std::cout << "   FWHM Y : " << hBegin->GetStdDev(1) * 2.35 << '\n';
    std::cout << "   FWHM Z : " << hBegin->GetStdDev(2) * 2.35 << '\n';
    std::cout << "-> End       : " << '\n';
    std::cout << "   FWHM Y : " << hEnd->GetStdDev(1) * 2.35 << '\n';
    std::cout << "   FWHM Z : " << hEnd->GetStdDev(2) * 2.35 << '\n';

    // Fit to get width in Z
    hBeginZ->Fit("gaus", "0QM+");
    auto* fit {hBeginZ->GetFunction("gaus")};
    if(fit)
    {
        auto* text {new TPaveText {0.5, 0.6, 0.7, 0.8, "NDC"}};
        text->SetBorderSize(0);
        text->AddText(TString::Format("#sigma = %.2f mm", fit->GetParameter(2)));
        fit->ResetBit(TF1::kNotDraw);
        hBeginZ->GetListOfFunctions()->Add(text);
    }


    // Plot
    auto* c0 {new TCanvas {"c0", "Emittance canvas"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hBegin->SetTitle("X = 0 mm");
    hBegin->DrawClone("colz");
    c0->cd(2);
    hEnd->SetTitle("X = 256 mm");
    hEnd->DrawClone("colz");
    c0->cd(3);
    hYthetaXY->DrawClone("colz");
    c0->cd(4);
    hYthetaXZ->DrawClone("colz");
    c0->cd(5);
    hZthetaXY->DrawClone("colz");
    c0->cd(6);
    hZthetaXZ->DrawClone("colz");

    auto* c1 {new TCanvas {"c1", "3D canvas"}};
    c1->DivideSquare(4);
    c1->cd(1);
    h3d->DrawClone();
    c1->cd(2);
    hxz.Merge()->DrawClone("colz");
    c1->cd(3);
    hyz.Merge()->DrawClone("colz");
    c1->cd(4);
    hBeginZ->DrawClone();
    // hDebugY->DrawClone("colz");
    // auto* f {new TF1 {"f", "0 + 1 * x", 80, 160}};
    // f->Draw("same");

    // Save objects to file
    auto file {std::make_unique<TFile>("./Outputs/histos.root", "recreate")};
    hBegin->Write("hBegin");
    hEnd->Write("hEnd");
    hxz->Write("hTrajXZ");
    hyz->Write("hTrajYZ");
    h3d->Write("h3d");
    hBeginZ->Write("hBeginZ");

    gSelector->SendToWebsite("emittance.root", c0, "cEmitt0");
    gSelector->SendToWebsite("emittance.root", c1, "cEmitt1");
}
