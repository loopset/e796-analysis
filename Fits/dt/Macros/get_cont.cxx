#include "ActKinematics.h"
#include "ActMergerData.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "FitInterface.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <memory>
#include <vector>

#include "../../../Selector/Selector.h"
#include "../../FitHist.h"
void get_cont()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", "../../../PostAnalysis/RootFiles/Pipe3/pd_as_dt_contamination.root"};
    auto hEx {df.Histo1D(E796Fit::Exdt, "Ex")};
    // (d,d) 1n PS as (d,t)
    ROOT::RDataFrame dps {"SimulationTTree", "../../../Simulation/Outputs/juan_RPx/tree_dd_1nps_as_dt.root"};
    auto hPS {dps.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr());

    // Read (d,t) data
    ROOT::RDataFrame dt {"Sel_Tree", "../../../PostAnalysis/RootFiles/Pipe3/tree_20O_2H_3H_front_juan_RPx.root"};
    auto hExdt {dt.Histo1D(E796Fit::Exdt, "Ex")};

    auto file {std::make_unique<TFile>("../Outputs/fit_juan_RPx.root")};
    auto* gfit {file->Get<TGraph>("GraphGlobal")};
    file->Close();

    // Get kinematic lines
    ROOT::RDF::TH2DModel mKin {"hKin", "Kinematics;#theta_{lab} [#circ];E_{lab} [MeV]", 300, 0, 60, 300, 0, 15};
    auto hKinpd {df.Histo2D(mKin, "fThetaLight", "EVertex")};
    auto hKindt {dt.Histo2D(mKin, "fThetaLight", "EVertex")};

    // Get evolution of contamination with cut in EVertex
    std::vector<TH1D*> hexs {};
    double emin {2};
    double emax {12};
    double estep {1};
    for(double e = emin; e <= emax; e += estep)
    {
        auto node {dt.Filter([=](ActRoot::MergerData& d) { return d.fSilEs.front() < e; }, {"MergerData"})};
        auto hEx {node.Histo1D(E796Fit::Exdt, "Ex")};
        auto* clone {(TH1D*)hEx->Clone()};
        clone->SetTitle(TString::Format("E < %.2f", e));
        hexs.push_back(clone);
    }

    auto* c0 {new TCanvas {"c0", "get pd cont"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hExdt->SetLineColor(8);
    hExdt->DrawNormalized();
    hEx->SetLineColor(kRed);
    hEx->DrawNormalized("same", 0.25);
    auto cte {hExdt->Integral()};
    gfit->Scale(1. / cte);
    gfit->SetLineWidth(1);
    gfit->SetLineColor(kMagenta);
    gfit->Draw("l");
    c0->cd(2);
    ActPhysics::Kinematics kin {"20O(d,t)@700|16"};
    auto* gdt {kin.GetKinematicLine3()};
    kin = ActPhysics::Kinematics {"20O(p,d)@700"};
    auto* gpd {kin.GetKinematicLine3()};
    hKindt->DrawClone("colz");
    hKinpd->DrawClone("colz same");
    gdt->SetLineColor(8);
    gdt->Draw("l");
    gpd->SetLineColor(kRed);
    gpd->Draw("l");

    auto* c1 {new TCanvas {"c1", "Evolution of contamination"}};
    c1->DivideSquare(hexs.size());
    for(int i = 0; i < hexs.size(); i++)
    {
        c1->cd(i + 1);
        gPad->SetLogy();
        hexs[i]->Draw();
    }

    // gSelector->SetFlag("juan_RPx");
    // Interpolators::Sigmas sigmas {gSelector->GetSigmasFile("2H", "3H").Data()};
    //
    // Fitters::Interface inter;
    // double sigma {0.364}; // common guess for all states
    // inter.AddState("g0", {400, 16.3, sigma}, "gs");
    // inter.AddState("g1", {250, 18.1, sigma}, "1st");
    // inter.AddState("g2", {110, 20.4, sigma}, "2nd");
    // inter.AddState("ps0", {0.1});
    // inter.EndAddingStates();
    // // Wider mean margin
    // inter.SetOffsetMeanBounds(0.5);
    // // Eval correct sigma
    // inter.EvalSigma(sigmas.GetGraph());
    // // inter.Write("./Outputs/inter.root");
    //
    //
    // // Fitting range
    // double exmin {-5};
    // double exmax {25};
    // // Model
    // Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS}};
    // // Run!
    // Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
    //                 ("./Outputs/test.root"), "20O(d,t) contamination");
}
