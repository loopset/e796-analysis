#include "TEfficiency.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMath.h"

#include <vector>

void xs()
{
    auto* fin {new TFile {"./graphs.root"}};
    auto* g0 {fin->Get<TGraphErrors>("g0")};
    auto* g1 {fin->Get<TGraphErrors>("g1")};

    double Nt {}; // number of target per cm2
    double Nb {}; // number of beams

    auto* eff {new TEfficiency {}};
    auto* finn {new TFile("./output_simulation.root")};
    auto* eff0 {finn->Get<TEfficiency>("eff")};
    eff->Draw();

    eff->Write("eff");
    // TMath::TwoPi() * (TMath::Cos(min * TMath::DegToRad()) - TMath::Cos(max * TMath::DegToRad()));

    std::vector<double> thetaCMs;
    auto* gOmega {new TGraphErrors};
    auto* gxs0 {new TGraphErrors};
    for(int p = 0; p < g0->GetN(); p++)
    {
        auto bin {eff->FindFixBin(thetaCMs[p])};
        auto epsilon {eff->GetEfficiency(bin)};
        auto Omega {gOmega->GetPointY(p)};

        double num {g0->GetPointY(p)};
        double denom {Nt * Nb * epsilon * Omega};

        auto xs {num / denom};
        xs *= 1e27;

        gxs0->SetPoint(p, thetaCMs[p], xs);
    }

    // Para avaliar eff
    // 1-> Atopamos o punto no Tefficiency correspondente ao noso centro
    auto thetaCMCentre {30};
    auto bin {eff->FindFixBin(thetaCMCentre)};
    auto eval {eff->GetEfficiency(bin)};


    double min {};
    auto bmin {eff->FindFixBin(min)};
    double max {};
    auto bmax {eff->FindFixBin(max)};
    std::vector<double> vals;
    for(int b = bmin; b < bmax; b++)
    {
        vals.push_back(eff->GetEfficiency(b));
    }
    auto epsilon {TMath::Mean(vals.begin(), vals.end())};
}
