#include "ActLine.h"
#include "ActTPCData.h"

#include "TCanvas.h"
#include "TH2F.h"

#include "Math/Point3D.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3D.h"

#include <cmath>
#include <iostream>
#include <vector>
ROOT::Math::XYZPointF Fit(const std::vector<ActRoot::Voxel>& voxels)
{
    double w {};
    double Sx {};
    double Sy {};
    double Sxx {};
    double Syy {};
    double Sxy {};
    for(const auto& v : voxels)
    {
        auto pos = v.GetPosition();
        pos += ROOT::Math::XYZVectorF{0.5, 0.5, 0.5}; 
        auto q {v.GetCharge()};
        Sx += pos.X() * q;
        Sxx += std::pow(pos.X(), 2) * q;
        Sxy += pos.X() * pos.Y() * q;
        Sy += pos.Y() * q;
        Syy += std::pow(pos.Y(), 2) * q;
        w += v.GetCharge();
    }

    double denom {w * Sxx - std::pow(Sx, 2)};
    double m {(w * Sxy - Sx * Sy) / denom};
    std::cout<<"w : "<<w<<'\n';
    std::cout<<"denom : "<<denom<<'\n';
    std::cout<<"m : "<<m<<'\n';
    return {(float)-1, (float)m, 0};
}

void testLine()
{
    double xmin {0};
    double xmax {8945};
    double y {64};
    double z {64};
    std::vector<ActRoot::Voxel> data;
    for(double x = xmin; x <= xmax; x += 1)
    {
        for(int i = 0; i < 2; i++)
        {
            data.push_back({{x, y + i, z}, 5});
        }
        // data.push_back({{x, x, z}, 5});
    }
    ActPhysics::Line line;
    line.FitVoxels(data);
    line.Print();
    auto other {Fit(data)};
    std::cout << "Other fit : " << other << '\n';

    auto* h {new TH2F("h", "Fit test", 128, 0, 128, 128, 0, 128)};
    for(const auto& voxel : data)
    {
        const auto& pos {voxel.GetPosition()};
        h->Fill(pos.X(), pos.Y());
    }

    auto* c {new TCanvas("c", "test line")};
    h->Draw("colz");
    auto p {line.GetPolyLine("xy")};
    p->DrawClone("same");
}
