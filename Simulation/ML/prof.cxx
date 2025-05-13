#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"

#include <iostream>
#include <string>
#include <vector>

#include "./simu.cxx"

void prof()
{
    std::string light {"1H"};

    auto* srim {new ActPhysics::SRIM};
    std::vector<std::string> parts {"1H", "2H", "3H"};
    std::string path {"./Inputs/SRIM/"};
    std::string filetag {"_900mb_CF4_90-10.txt"};
    for(const auto& part : parts)
        srim->ReadTable(part, path + part + filetag);
    srim->Draw();

    // Histograms
    auto* hProf {new TProfile {"hProf", "Q profile;dist [mm];#DeltaE [MeV]", 200, 0, 300}};
    std::vector<TProfile*> hs;
    for(const auto& part : parts)
    {
        auto* clone {(TProfile*)hProf->Clone()};
        clone->SetNameTitle(TString::Format("h%s", part.c_str()), part.c_str());
        clone->Print();
        hs.push_back(clone);
    }

    // Energy parameters
    double Tini {20};
    // Vertex
    XYZPoint vertex {50, 128, 128};
    // Direction
    XYZVector dir {1, 0, 0};
    for(int i = 0; i < parts.size(); i++)
        FillProfile(srim, parts[i], Tini, vertex, dir, hs[i]);

    auto* c0 {new TCanvas {"c0", "Profile canvas"}};
    c0->DivideSquare(4);
    for(int i = 0; i < parts.size(); i++)
    {
        c0->cd(i + 1);
        hs[i]->Draw("histe");
    }
}
