#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TLine.h"
#include "TString.h"
#include "TVirtualPad.h"

#include <memory>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"
void GetProfile()
{
    // Set value
    double driftfactor {2.344};

    // Read data
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, true)};

    // Book
    auto hEx {df.Histo1D(HistConfig::Ex, "Ex")};
    auto hExZ {df.Histo2D(HistConfig::ExRPZ, "fSP.fCoordinates.fZ", "Ex")};
    // Fit g.s
    double gswidth {2};
    hEx->Fit("gaus", "0Q+", "", -gswidth / 2, + gswidth / 2);
    auto* gaus {hEx->GetFunction("gaus")};
    gaus->ResetBit(TF1::kNotDraw);
    // Profile
    double ex {gaus->GetParameter("Mean")};
    double exwidth {5 * gaus->GetParameter("Sigma")}; // MeV
    double exmin {ex - exwidth / 2};
    double exmax {ex + exwidth / 2};
    auto bmin {hExZ->GetYaxis()->FindBin(exmin)};
    auto bmax {hExZ->GetYaxis()->FindBin(exmax)};
    auto* hProf {hExZ->ProfileX("hProfX", bmin, bmax)};
    hProf->SetTitle(TString::Format("E_{x} profile #in [%.1f, %.1f] MeV", exmin, exmax));
    // Fit
    TString funcstr {"pol2"};
    hProf->Fit(funcstr, "0Q+");
    hProf->GetFunction(funcstr)->ResetBit(TF1::kNotDraw);

    // Draw
    auto* c0 {new TCanvas {"c0", "Drift correction canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hEx->DrawClone();
    c0->cd(2);
    hExZ->DrawClone("colz");
    gPad->Update();
    auto* lmin {new TLine {gPad->GetUxmin(), exmin, gPad->GetUxmax(), exmin}};
    auto* lmax {new TLine {gPad->GetUxmin(), exmax, gPad->GetUxmax(), exmax}};
    for(auto* l : {lmin, lmax})
    {
        l->SetLineWidth(2);
        l->SetLineColor(kMagenta);
        l->Draw("same");
    }
    c0->cd(3);
    hProf->Draw();

    // Save things
    TString path {"./Outputs/"};
    auto name {TString::Format("_%s_%s_%s_%s_drift_%.4f.root", gSelector->GetBeam().c_str(), gSelector->GetTarget().c_str(),
                               gSelector->GetLight().c_str(), gSelector->GetFlag().c_str(), driftfactor)};
    df.Snapshot("Sel_Tree", (path + "df" + name).Data());
    auto file {std::make_unique<TFile>(path + "hs" + name, "recreate")};
    hExZ->Write();
    hProf->Write();
}
