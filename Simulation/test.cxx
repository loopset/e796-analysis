#include "ActCrossSection.h"
#include "ActKinematics.h"

#include "TH1.h"

#include <iostream>
// #include "ActSilSpecs.h"
//
// #include "TCanvas.h"
// #include "TH2D.h"
// #include "TMath.h"
// #include "TRandom.h"
//
// #include "Math/Point3Dfwd.h"
// #include "Math/Vector3Dfwd.h"
//
// #include <iomanip>
// #include <iostream>
//
// #include "../PostAnalysis/HistConfig.h"
// #include "../PostAnalysis/Utils.cxx"
//
void test()
{
    // Cross-section
    ActSim::CrossSection* xs {};
    xs = new ActSim::CrossSection;
    xs->ReadData("./Inputs/pp.dat");
    xs->DrawCDF();
    xs->DrawTheo();
    ActPhysics::Kinematics("20O(p,p)@700").Draw();

    auto* h1 {new TH1D {"h", "Sampling", 300, 0, 180}};
    for(int i = 0; i < 10000; i++)
        h1->Fill(xs->Sample());

    h1->Draw();
    // ActPhysics::SilSpecs specs {};
    // specs.ReadFile("../configs/detailedSilicons.conf");
    // // specs.ReplaceWithMatrix("l0", E796Utils::GetSideMatrix());
    // specs.GetLayer("l0").Print();
    // // specs.ReadFile("../configs/detailedSilicons.conf");
    // // specs.EraseLayer("f0");
    // // specs.EraseLayer("f1");
    //
    // auto hSP {HistConfig::SP.GetHistogram()};
    //
    // for(int i = 0; i < 1000000; i++)
    // {
    //     // Vertex
    //     ROOT::Math::XYZPoint vertex {gRandom->Uniform() * 256, 256. / 2, 256. / 2};
    //     // Direction
    //     auto theta3Lab {TMath::ACos(gRandom->Uniform(-1, 1))};
    //     auto phi3Lab {TMath::TwoPi() * gRandom->Uniform()};
    //     ROOT::Math::XYZVector dir {TMath::Cos(theta3Lab), TMath::Sin(theta3Lab) * TMath::Sin(phi3Lab),
    //                                TMath::Sin(theta3Lab) * TMath::Cos(phi3Lab)};
    //     auto [idx, sp] {specs.FindSPInLayer("l0", vertex, dir)};
    //     if(idx != -1)
    //         hSP->Fill(sp.X(), sp.Z());
    // }
    //
    // // Draw
    // auto* c0 {new TCanvas {"c0", "Testing new simu"}};
    // hSP->DrawClone("colz");
    // specs.GetLayer("l0").GetSilMatrix()->DrawClone();
}
