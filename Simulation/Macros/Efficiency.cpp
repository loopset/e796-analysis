#include "ROOT/RDF/HistoModels.hxx"
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
    std::string target {"1H"};
    std::string light {"2H"};

    // Get analysis histogram
    ROOT::RDataFrame da {"Final_Tree", E796Utils::GetFileName(2, beam, target, light, false)};
    // Fill Ex histogram
    auto hEx {da.Histo1D(HistConfig::Ex, "Ex")};
    // Fill all correlation histogram
    auto ha {da.Histo2D(HistConfig::RPxThetaCM, "fRP.fCoordinates.fX", "ThetaCM")};
    ha->SetNameTitle("pana", "Ana all E_{x}");
    // But RP depends strongly on excited state...
    std::vector<double> Eexs {0, 3.24};
    double sigma {1};
    std::vector<TH2D*> has;
    auto* as {new THStack};
    as->SetTitle("Analysis RP.X() per E_{x};X [mm];Normalized counts");
    // Add curve of Ex vs RPx for analysis
    TH2D* hags {};
    ROOT::RDF::TH2DModel hexgs {"hExGS", "E_{x} vs RP.X for gs;RP.X [mm];E_{x} [MeV]", 200, 0, 300, 50, -2, 2};
    for(const auto& ex : Eexs)
    {
        auto gated {da.Filter([=](double e) { return std::abs(ex - e) <= sigma; }, {"Ex"})};
        auto haux {gated.Histo2D(HistConfig::RPxThetaCM, "fRP.fCoordinates.fX", "ThetaCM")};
        if(ex == 0)
        {
            auto hgs {gated.Histo2D(hexgs, "fRP.fCoordinates.fX", "Ex")};
            hags = (TH2D*)hgs->Clone();
        }
        haux->SetTitle(TString::Format("Ana E_{x} = %.2f MeV", ex));
        has.push_back((TH2D*)haux->Clone());
        as->Add(has.back(), "col");
    }
    // Stack for analysis Ex and Ex RPx gs
    auto* sags {new THStack};
    sags->Add(hEx.GetPtr());
    sags->Add(hags, "col");

    // Get simulation histogram
    auto* stsimu {new THStack};
    auto* stkin {new THStack};
    std::vector<TH2D*> hss, hskin;
    for(const auto& ex : Eexs)
    {
        ROOT::RDataFrame df {"SimulationTTree", E796Simu::GetFile(beam, target, light, ex)};
        auto haux {df.Histo2D(HistConfig::RPxThetaCM, "RPx", "theta3CM")};
        auto hkin {df.Histo2D(HistConfig::KinCM, "theta3CM", "EVertex")};
        haux->SetTitle(TString::Format("Simu E_{x} = %.2f MeV", ex));
        hkin->SetTitle(TString::Format("Simu kin E_{x} = %.2f MeV", ex));
        hss.push_back((TH2D*)haux->Clone());
        hskin.push_back((TH2D*)hkin->Clone());
        stsimu->Add(hss.back(), "col");
        stkin->Add(hskin.back(), "col");
    }
    // ROOT::RDataFrame ds {"SimulationTTree", E796Simu::GetFile(beam, target, light, 3.24)};
    // auto psimu {ds.Histo1D(
    //     {"psimu", "Simu E_{x} = 3.24;X [mm]", HistConfig::RP.fNbinsX, HistConfig::RP.fXLow, HistConfig::RP.fXUp},
    //     "RPx")};
    // ROOT::RDataFrame dss {"SimulationTTree", "./Outputs/Eff_study/d_t_gs_sampled_rpx.root"};
    // auto psimus {dss.Histo1D(
    //     {"psimus", "Simu samp gs;X [mm]", HistConfig::RP.fNbinsX, HistConfig::RP.fXLow, HistConfig::RP.fXUp},
    //     "RPx")};
    // auto hXThetaCM {ds.Histo2D(
    //     {"hXThetaCM", "RP.X vs #theta_{CM} correlation;#theta_{CM} [#circ];RP.X [mm]", 300, 0, 60, 400, 0, 300},
    //     "theta3CM", "RPx")};

    // // Add to stack
    // auto* stack {new THStack};
    // stack->SetTitle(";X [mm];Normalized counts");
    // // Normalize and set style
    // for(auto* h : {pana, psimu.GetPtr(), psimus.GetPtr()})
    // {
    //     h->Scale(1. / h->Integral());
    //     h->SetLineWidth(2);
    //     stack->Add(h, "hist");
    // }

    // Get efficiency
    Interpolators::Efficiency effs;
    effs.Add("Sampled RP.X", "./Outputs/Eff_study/d_t_gs_sampled_rpx.root");
    effs.Add("Free RP.X", "./Outputs/Eff_study/d_t_gs_free_rpx.root");
    effs.Draw();

    // Draw
    auto* c0 {new TCanvas {"c0", "RP X comparison"}};
    c0->DivideSquare(4);
    c0->cd(1);
    sags->DrawClone("pads");
    c0->cd(2);
    as->Draw("pads");
    c0->cd(3);
    stsimu->Draw("pads");
    c0->cd(4);
    stkin->Draw("pads");


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
