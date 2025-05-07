#include "ActSRIM.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TH1.h"
#include "TMath.h"
#include "TRandom.h"
#include "TString.h"
#include "TStyle.h"

#include <string>
#include <vector>

double GetStragg(ActPhysics::SRIM* srim, const std::string& name, double e, double t)
{
    std::cout << "=================" << '\n';
    auto Rini {srim->EvalRange(name, e)};
    auto uRini {srim->EvalLongStraggling(name, Rini)};
    std::cout<<"Rini : "<<Rini<<'\n';
    std::cout << "uRini : " << uRini << '\n';
    auto Rafter {Rini - t};
    if(Rafter <= 0)
        return 0;
    auto uRafter {srim->EvalLongStraggling(name, Rafter)};
    std::cout << "uRafter : " << uRafter << '\n';
    auto udist {TMath::Sqrt(uRini * uRini - uRafter * uRafter)};
    std::cout<<"dist : "<<t<<'\n';
    std::cout<<"udist : "<<udist<<'\n';
    t = gRandom->Gaus(t, udist);
    std::cout<<"random dist : "<<t<<'\n';
    return t;
}

void test()
{
    auto* srim {new ActPhysics::SRIM};
    // srim->ReadTable("d", "../Calibrations/SRIMData/raw/2H_silicon.txt");
    srim->ReadTable("d", "./ML/Inputs/SRIM/11Li_silicon.txt");

    // std::vector<double> es {5, 10, 15, 20};
    // std::vector<double> es {20, 30, 40, 50, 60};
    std::vector<double> es {30};
    auto thick {60.e-3}; // mm
    auto sigma {1.e-3};  // mm

    std::vector<TH1D*> hs, hus;
    for(const auto& e : es)
    {
        auto* hdE {new TH1D {TString::Format("hdE%.0f", e), TString::Format("T_{ini} = %.2f MeV;#DeltaE [MeV]", e), 400,
                             0, 20}};
        hs.push_back(hdE);
        auto* hu {new TH1D {TString::Format("hU%.0f", e), TString::Format("T_{ini} = %.2f MeV;#Delta d [#mum]", e), 200,
                            0, 2 * thick * 1e3}};
        hus.push_back(hu);
    }

    int iter {100000};
    int idx {};
    for(const auto& e : es)
    {
        for(int i = 0; i < iter; i++)
        {
            auto unc {GetStragg(srim, "d", e, thick)};
            // manual computation
            hus[idx]->Fill(unc * 1e3);
            // auto t {unc};
            auto t {gRandom->Gaus(thick, 0.0035 / 2/ 3)};
            // auto t {thick};
            auto eafter {srim->Slow("d", e, t)};
            auto eloss {e - eafter};
            hs[idx]->Fill(eloss);
        }
        idx++;
    }

    // Draw
    gStyle->SetOptFit();
    auto* c0 {new TCanvas {"c0", "test"}};
    c0->DivideSquare(hs.size());
    for(int i = 0; i < hs.size(); i++)
    {
        c0->cd(i + 1);
        hs[i]->Fit("gaus", "0Q");
        auto* func {hs[i]->GetFunction("gaus")};
        if(func)
            hs[i]->GetFunction("gaus")->ResetBit(TF1::kNotDraw);
        hs[i]->Draw();
    }
    auto* c1 {new TCanvas {"c1", "straggling"}};
    c1->DivideSquare(hus.size());
    for(int i = 0; i < hus.size(); i++)
    {
        c1->cd(i + 1);
        hus[i]->Draw();
    }
}
