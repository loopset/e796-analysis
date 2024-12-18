#include "ActSilSpecs.h"

#include "TCanvas.h"
#include "TH2D.h"
#include "TMath.h"
#include "TRandom.h"

#include "Math/Point3Dfwd.h"
#include "Math/Vector3Dfwd.h"

#include <iomanip>
#include <iostream>

#include "../PostAnalysis/HistConfig.h"
#include "../PostAnalysis/Utils.cxx"

void test()
{
    ActPhysics::SilSpecs specs {};
    specs.ReadFile("../configs/detailedSilicons.conf");
    // specs.ReplaceWithMatrix("l0", E796Utils::GetSideMatrix());
    specs.GetLayer("l0").Print();
    // specs.ReadFile("../configs/detailedSilicons.conf");
    // specs.EraseLayer("f0");
    // specs.EraseLayer("f1");

    auto hSP {HistConfig::SP.GetHistogram()};

    for(int i = 0; i < 1000000; i++)
    {
        // Vertex
        ROOT::Math::XYZPoint vertex {gRandom->Uniform() * 256, 256. / 2, 256. / 2};
        // Direction
        auto theta3Lab {TMath::ACos(gRandom->Uniform(-1, 1))};
        auto phi3Lab {TMath::TwoPi() * gRandom->Uniform()};
        ROOT::Math::XYZVector dir {TMath::Cos(theta3Lab), TMath::Sin(theta3Lab) * TMath::Sin(phi3Lab),
                                   TMath::Sin(theta3Lab) * TMath::Cos(phi3Lab)};
        auto [idx, sp] {specs.FindSPInLayer("l0", vertex, dir)};
        if(idx != -1)
            hSP->Fill(sp.X(), sp.Z());
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Testing new simu"}};
    hSP->DrawClone("colz");
    specs.GetLayer("l0").GetSilMatrix()->DrawClone();
}
