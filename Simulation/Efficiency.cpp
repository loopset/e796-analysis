#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TH1.h"
#include "THStack.h"
#include "TROOT.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "Interpolators.h"

#include <string>
#include <vector>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"
#include "/media/Data/E796v2/Simulation/Utils.cxx"

void Efficiency()
{
    ROOT::EnableImplicitMT();

    std::string beam {"20O"};
    std::string target {"2H"};
    std::string light {"3H"};

    // Get analysis histogram
    ROOT::RDataFrame da {"Final_Tree", E796Utils::GetFileName(2, beam, target, light, false)};
    // Fill Ex histogram
    auto hEx {da.Histo1D(HistConfig::Ex, "Ex")};
    // RP histogram
    auto hRPa {da.Histo2D(HistConfig::RP, "fRP.fCoordinates.fX", "fRP.fCoordinates.fY")};
    auto* pana {hRPa->ProjectionX()};
    pana->SetNameTitle("pana", "Analysis");
    // But RP depends strongly on excited state...
    std::vector<double> Eexs {0, 3.2};
    double sigma {1};
    std::vector<TH1D*> panas;
    auto* as {new THStack};
    as->SetTitle("Analysis RP.X() per E_{x};X [mm];Normalized counts");
    for(const auto& ex : Eexs)
    {
        auto gated {da.Filter([=](double e) { return std::abs(ex - e) <= sigma; }, {"Ex"})};
        auto hrp {gated.Histo2D(HistConfig::RP, "fRP.fCoordinates.fX", "fRP.fCoordinates.fY")};
        panas.push_back(hrp->ProjectionX(TString::Format("pana%.2f", ex)));
        auto& p {panas.back()};
        p->SetTitle(TString::Format("E_{x} = %.2f", ex));
        // Normalize
        p->Scale(1. / panas.back()->Integral());
        p->SetLineWidth(2);
        p->Smooth(5);
        as->Add(panas.back(), "hist");
    }

    // Get simulation histogram
    ROOT::RDataFrame ds {"SimulationTTree", E796Simu::GetFile(beam, target, light, 0)};
    auto psimu {ds.Histo1D(
        {"psimu", "Simulation;X [mm]", HistConfig::RP.fNbinsX, HistConfig::RP.fXLow, HistConfig::RP.fXUp}, "RPx")};
    ROOT::RDataFrame dss {"SimulationTTree", "./Outputs/Eff_study/d_t_gs_sampled_rpx.root"};
    auto psimus {dss.Histo1D(
        {"psimus", "Simu sampled;X [mm]", HistConfig::RP.fNbinsX, HistConfig::RP.fXLow, HistConfig::RP.fXUp}, "RPx")};

    // Add to stack
    auto* stack {new THStack};
    stack->SetTitle(";X [mm];Normalized counts");
    // Normalize and set style
    for(auto* h : {pana, psimu.GetPtr(), psimus.GetPtr()})
    {
        h->Scale(1. / h->Integral());
        h->SetLineWidth(2);
        stack->Add(h, "hist");
    }

    // Get efficiency
    Interpolators::Efficiency effs;
    effs.Add("Sampled RP.X", "./Outputs/Eff_study/d_t_gs_sampled_rpx.root");
    effs.Add("Free RP.X", "./Outputs/Eff_study/d_t_gs_free_rpx.root");
    effs.Draw();

    // Draw
    auto* c0 {new TCanvas {"c0", "RP X comparison"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hEx->DrawClone();
    c0->cd(2);
    as->Draw("nostack plc");
    gPad->BuildLegend();
    c0->cd(3);
    stack->DrawClone("nostack plc");
    c0->cd(3)->BuildLegend();


    // Interpolators::Efficiency effs;
    // effs.Add("(d,t) gs RP.X < 128", "./Outputs/Eff_study/d_t_gs_1.root");
    // effs.Add("(d,t) gs RP.X > 128", "./Outputs/Eff_study/d_t_gs_2.root");
    // effs.Add("(d,t) gs all", "./Outputs/Eff_study/d_t_gs_all.root");
    // effs.Draw(true);
    //
    // Interpolators::Efficiency effs1;
    // effs1.Add("(p,d) gs RP.X < 128", "./Outputs/Eff_study/p_d_gs_1.root");
    // effs1.Add("(p,d) gs RP.X > 128", "./Outputs/Eff_study/p_d_gs_2.root");
    // effs1.Add("(p,d) gs all", "./Outputs/Eff_study/p_d_gs_all.root");
    // effs1.Draw(true);
}
