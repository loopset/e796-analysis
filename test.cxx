#include "ActSRIM.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"

#include <fstream>
#include <iostream>
#include <utility>

std::pair<TGraph*, TGraph*> Init(const std::string& file)
{
    std::ifstream streamer {file};
    double e {};
    double esp {};
    double nsp {};
    double r {};
    double ls {};
    double as {};
    auto* ge {new TGraph};
    auto* gr {new TGraph};
    while(streamer >> e >> esp >> nsp >> r >> ls >> as)
    {
        ge->SetPoint(ge->GetN(), e, r);
        gr->SetPoint(gr->GetN(), r, e);
    }
    for(auto ptr : {ge, gr})
    {
        ptr->SetLineWidth(2);
        ptr->SetMarkerStyle(24);
    }
    return {ge, gr};
}

double Slow(double e, double t, TGraph* ge, TGraph* gr)
{
    auto Rini {ge->Eval(e, nullptr, "S")};
    std::cout << "===================" << '\n';
    std::cout << "Eini : " << e << " rini : " << Rini << '\n';
    auto RLeft {Rini - t};
    if(RLeft <= 0)
        return 0;
    auto ret {gr->Eval(RLeft, nullptr, "S")};
    std::cout << "rleft : " << RLeft << " Eleft : " << ret << '\n';
    return ret;
}

void test()
{
    ActPhysics::SRIM srim;
    srim.ReadTable("p", "./Calibrations/SRIMData/raw/1H_silicon.txt");
    srim.ReadInterpolations("pOld", "./Calibrations/SRIMData/transformed/1H_silicon.dat");

    auto [ge, gr] {Init("./Calibrations/SRIMData/transformed/1H_silicon.dat")};

    srim.Draw();
    // ola que tal estamos
    // ola aslladk double double double ol what is this ola
    // ola
    /*
ola que tal disabked here
    */

    std::string a = "abc"
                    "cde";

    // Set thickness of must2 dssd
    double thick {300 * 1.e-3}; // mm
    // Multigraph to store results
    auto* mg {new TMultiGraph};
    // Settings of energy
    double emin {5};
    double emax {150};
    double estep {1};
    for(const auto& name : {"p", "pOld"})
    {
        auto* g {new TGraph};
        g->SetTitle(name);
        for(double e = emin; e <= emax; e += estep)
        {
            // auto eres {srim.Slow(name, e, thick)};
            auto eres {Slow(e, thick, ge, gr)};
            auto deltae {e - eres};
            std::cout << "Delta E : " << deltae << '\n';
            if(eres <= 0)
                continue;
            g->SetPoint(g->GetN(), eres, deltae);
        }
        // Add to mg
        g->SetMarkerStyle(24);
        g->SetLineWidth(2);
        mg->Add(g);
    }

    // Save it
    mg->SetNameTitle("mg", "SRIM #Delta E - E;E [MeV];#Delta E [MeV]");

    // Draw
    auto* c0 {new TCanvas {"c0", "SRIM canvas"}};
    mg->Draw("apl plc pmc");
    auto* leg {c0->BuildLegend()};

    auto* c1 {new TCanvas {"c1", "Debug canvas"}};
    c1->DivideSquare(2);
    c1->cd(1);
    ge->Draw("apl");
    c1->cd(2);
    gr->Draw("apl");
}
